// LLM Module - Manages llama-server (QProcess) and formats transcriptions over
// HTTP.

#include "llm.hpp"

#include <QCoreApplication>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QThread>

#include "../log/log.hpp"
#include "../setting/setting.hpp"

#ifdef Q_OS_WIN
#define LLAMA_SERVER_BINARY "llama-server.exe"
#else
#define LLAMA_SERVER_BINARY "llama-server"
#endif

static const char* SERVER_URL = "http://127.0.0.1:8080/v1/chat/completions";

static QString build_prompt(const QString& text) {
  Setting* s = Setting::instance();
  QStringList rules;
  int ruleNum = 1;

  rules << QString(
    "%1. Apply proper casing: capitalize the first word of "
    "each sentence and proper nouns.")
             .arg(ruleNum++);

  if (s->allow_punctuation()) {
    rules << QString(
      "%1. Add proper punctuation (periods, commas, question "
      "marks, etc.) where grammatically required.")
               .arg(ruleNum++);
  } else {
    rules << QString(
      "%1. Do NOT add any punctuation. Output without periods, "
      "commas, or any punctuation marks.")
               .arg(ruleNum++);
  }

  if (!s->allow_accents()) {
    rules << QString(
      "%1. Remove all accents and diacritical marks from "
      "characters (e.g. é -> e, ñ -> n).")
               .arg(ruleNum++);
  }

  if (!s->markdown_mode()) {
    rules << QString(
      "%1. NEVER use Markdown, bullet points, bold, italics, "
      "headers, code blocks, or any formatting syntax. Output "
      "plain text only.")
               .arg(ruleNum++);
  }

  const auto dict = s->dictionary();
  for (auto it = dict.cbegin(); it != dict.cend(); ++it) {
    rules << QString("%1. Replace the word \"%2\" with \"%3\".")
               .arg(QString::number(ruleNum++), it.key(), it.value());
  }

  rules << QString(
    "%1. Do not add, remove, or rephrase any words. Preserve the "
    "exact spoken content.")
             .arg(ruleNum++);
  rules << QString(
    "%1. Return ONLY the formatted transcription. No greetings, no "
    "explanations, no commentary.")
             .arg(ruleNum++);

  return "You are a speech transcription formatter. Your sole task is to "
         "take "
         "raw speech-to-text output and produce a clean, readable "
         "transcription. "
         "Strict rules:\n" +
    rules.join("\n") + "\n\nRaw text:\n" + text;
}

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

LLM::LLM(QObject* parent) : QObject(parent), m_is_running(false) {}

LLM::~LLM() { stop(); }

bool LLM::start() {
  if (m_is_running) {
    return true;
  }

  QString dir = QCoreApplication::applicationDirPath() + "/llama.cpp/llama-b9265";
  QString program = dir + "/" + LLAMA_SERVER_BINARY;
  if (!QFileInfo::exists(program)) {
    qCCritical(lcLlm) << "llama-server not found at:" << program;
    return false;
  }

  m_server.setProgram(program);
  m_server.setArguments(QStringList()
    << "-m" << dir + "/models/Llama-3.2-1B-Instruct-Q4_K_M.gguf" << "--ctx-size"
    << "8192"
    << "--temperature" << "0.25"
    << "--top-k" << "20"
    << "--reasoning" << "off"
    << "--reasoning-budget" << "0"
    << "--n-gpu-layers" << "99"
    << "-ngl" << "99"
    << "--port" << "8080");

  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert("LD_LIBRARY_PATH", dir + ":" + env.value("LD_LIBRARY_PATH"));
  m_server.setProcessEnvironment(env);

  m_server.start();
  if (!m_server.waitForStarted(5000)) {
    qCCritical(lcLlm) << "failed to start llama-server";
    return false;
  }

  // Startup only (before the UI is shown): give the server time to load the
  // model
  QThread::sleep(2);
  m_is_running = true;
  qCInfo(lcLlm) << "started, server ready";
  return true;
}

void LLM::stop() {
  if (m_server.state() != QProcess::NotRunning) {
    m_server.terminate();
    if (!m_server.waitForFinished(3000)) {
      m_server.kill();
      m_server.waitForFinished(1000);
    }
  }
  m_is_running = false;
  qCInfo(lcLlm) << "stopped";
}

// Async: emits FORMATTED_TEXT_READY when the server replies. On any failure
// the raw text is emitted, so the user never loses their dictation.
void LLM::format_transcription(const QString& text) {
  if (!m_is_running || text.isEmpty()) {
    emit FORMATTED_TEXT_READY(text);
    return;
  }

  qCInfo(lcLlm) << "request:" << text.length() << "chars:" << text;

  QJsonObject message;
  message["role"] = "user";
  message["content"] = build_prompt(text);

  QJsonObject json;
  json["messages"] = QJsonArray({message});
  json["temperature"] = 0.0;

  QNetworkRequest request{QUrl(SERVER_URL)};
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  QNetworkReply* reply = m_network.post(request, QJsonDocument(json).toJson());

  connect(reply, &QNetworkReply::finished, this, [this, reply, text]() {
    reply->deleteLater();
    QString result;
    if (reply->error() == QNetworkReply::NoError) {
      result = parse_content(reply->readAll());
    } else {
      qCCritical(lcLlm) << "request failed:" << reply->errorString();
    }
    qCInfo(lcLlm) << "response:" << result.length() << "chars:" << result;
    emit FORMATTED_TEXT_READY(result.isEmpty() ? text : result);
  });
}
