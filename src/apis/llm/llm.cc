// LLM - Owns llama-server (QProcess) and formats transcriptions. Runs inside the gladys-llm process.

#include "llm.hpp"

#include <QCoreApplication>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

#include "../../modules/log/log.hpp"
#include "prompt.hpp"

#ifdef Q_OS_WIN
#define LLAMA_SERVER_BINARY "llama-server.exe"
#else
#define LLAMA_SERVER_BINARY "llama-server"
#endif

static const char* SERVER_URL = "http://127.0.0.1:8080/v1/chat/completions";
static const char* HEALTH_URL = "http://127.0.0.1:8080/health";  // 503 while loading, 200 ready
static const int HEALTH_INTERVAL_MS = 250;
static const int SERVER_START_TIMEOUT_MS = 5000;

// Returns the message content, or an empty string when the reply is malformed.
static QString parse_content(const QByteArray& body) {
  QJsonDocument doc = QJsonDocument::fromJson(body);
  if (!doc.isObject()) {
    return QString();
  }
  QJsonArray choices = doc.object()["choices"].toArray();
  if (choices.isEmpty()) {
    return QString();
  }
  return choices[0].toObject()["message"].toObject()["content"].toString();
}

// Literal, deterministic replacements applied to the LLM output.
static QString apply_snippets(QString text, const QVariantMap& snippets) {
  for (auto it = snippets.cbegin(); it != snippets.cend(); ++it) {
    text.replace(it.key(), it.value().toString());
  }
  return text;
}

LLM::LLM(QObject* parent) : QObject(parent) {
  m_health_timer.setInterval(HEALTH_INTERVAL_MS);
  connect(&m_health_timer, &QTimer::timeout, this, &LLM::poll_health);
}

LLM::~LLM() { stop(); }

bool LLM::start() {
  QString dir = QCoreApplication::applicationDirPath() + "/llama.cpp/llama-b9265";
  QString model = dir + "/models/Llama-3.2-1B-Instruct-Q4_K_M.gguf";
  QString program = dir + "/" + LLAMA_SERVER_BINARY;
  if (!QFileInfo::exists(program)) {
    qCCritical(lcLlm) << "llama-server not found at:" << program;
    return false;
  }

  m_server.setProgram(program);
  m_server.setArguments(QStringList() << "--model" << model << "--ctx-size" << "4096"
                                      << "--temperature" << "0.7"
                                      << "--top-p" << "0.9"
                                      << "--reasoning" << "off"
                                      << "--reasoning-budget" << "0"
                                      << "--n-gpu-layers" << "16"
                                      << "--flash-attn" << "on"
                                      << "--cache-type-k" << "q4_0"
                                      << "--repeat-penalty" << "1.15"
                                      << "--repeat-last-n" << "16"
                                      << "--port" << "8080");

  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert("LD_LIBRARY_PATH", dir + ":" + env.value("LD_LIBRARY_PATH"));
  m_server.setProcessEnvironment(env);
  m_server.setProcessChannelMode(QProcess::ForwardedErrorChannel);

  // Child exit is fatal; the parent (core) restarts this process.
  connect(&m_server, &QProcess::finished, this, []() {
    qCCritical(lcLlm) << "llama-server exited";
    QCoreApplication::exit(1);
  });

  m_server.start();
  if (!m_server.waitForStarted(SERVER_START_TIMEOUT_MS)) {
    qCCritical(lcLlm) << "failed to start llama-server";
    return false;
  }
  m_health_timer.start();
  return true;
}

void LLM::stop() {
  m_health_timer.stop();
  QObject::disconnect(&m_server, &QProcess::finished, nullptr, nullptr);
  if (m_server.state() != QProcess::NotRunning) {
    m_server.terminate();
    if (!m_server.waitForFinished(3000)) {
      m_server.kill();
      m_server.waitForFinished(1000);
    }
  }
  m_is_ready = false;
  qCInfo(lcLlm) << "stopped";
}

bool LLM::is_ready() const { return m_is_ready; }

void LLM::poll_health() {
  QNetworkReply* reply = m_network.get(QNetworkRequest(QUrl(HEALTH_URL)));
  connect(reply, &QNetworkReply::finished, this, [this, reply]() {
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
      return;  // still loading (503) or not listening yet
    }
    m_health_timer.stop();
    m_is_ready = true;
    qCInfo(lcLlm) << "llama-server ready";
  });
}

// Async: done runs when the server replies. On any failure it gets the raw text,
// so the user never loses their dictation.
void LLM::format(const QString& text, const QVariantMap& settings, Done done) {
  if (!m_is_ready || text.isEmpty()) {
    done(text);
    return;
  }

  qCInfo(lcLlm) << "request:" << text.length() << "chars:" << text;

  const QVariantMap snippets = settings.value("snippets").toMap();

  QJsonObject system;
  system["role"] = "system";
  system["content"] = build_system_prompt(settings.value("style").toString(),
                                          settings.value("markdown").toBool(),
                                          settings.value("dictionary").toMap());

  QJsonObject user;
  user["role"] = "user";
  user["content"] =
    "TASK\nFormat the input according to the instructions & examples above.\n"
    "input: " +
    text + "\noutput: ";

  QJsonObject json;
  json["messages"] = QJsonArray({system, user});
  json["stop"] = QJsonArray({"\ninput:", "\n\n", "\n \n"});
  json["max_tokens"] = static_cast<int>(text.length() + 10);

  QNetworkRequest request{QUrl(SERVER_URL)};
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  QNetworkReply* reply = m_network.post(request, QJsonDocument(json).toJson());

  connect(reply, &QNetworkReply::finished, this, [reply, text, snippets, done]() {
    reply->deleteLater();
    QString result;
    if (reply->error() == QNetworkReply::NoError) {
      result = apply_snippets(parse_content(reply->readAll()), snippets);
    } else {
      qCCritical(lcLlm) << "request failed:" << reply->errorString();
    }
    qCInfo(lcLlm) << "response:" << result.length() << "chars:" << result;
    done(result.isEmpty() ? text : result);
  });
}
