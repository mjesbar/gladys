// Core - Pipeline and supervisor for gladys-asr and gladys-llm (llama-server). Runs on the GUI thread.

#include "core.hpp"

#include <QCoreApplication>
#include <QFileInfo>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QUrl>
#include <cstdio>

#include "apis/llm/llm.hpp"
#include "apis/server.hpp"
#include "modules/data/data.hpp"
#include "modules/keypaste/paste.hpp"
#include "modules/log/log.hpp"

#ifdef Q_OS_WIN
#define EXE_SUFFIX ".exe"
#else
#define EXE_SUFFIX ""
#endif

static const int RESTART_DELAY_MS = 1000;
static const int STOP_TIMEOUT_MS = 6000;
static const int PROBE_INTERVAL_MS = 250;
static const int REQUEST_TIMEOUT_MS = 30000;  // a stop includes the transcription
static const int LLM_TO_COPY_DELAY_MS = 10;
static const int COPY_TO_PASTE_DELAY_MS = 250;

static QString sibling_binary(const char* name) {
  return QCoreApplication::applicationDirPath() + "/" + name + EXE_SUFFIX;
}

static QString api_url(quint16 port) { return QStringLiteral("http://127.0.0.1:%1").arg(port); }

static const QRegularExpression ANSI_COLOR("\x1b\\[[0-9;]*m");
static const QRegularExpression SEVERE_LEVEL("\\b(ERROR|FATAL)\\b");
static const QRegularExpression WARN_LEVEL("\\bWARN\\b");

Core::Core(const QString& model_dir, Paste* paste, QObject* parent)
    : QObject(parent), m_paste(paste) {
  m_asr.name = "gladys-asr";
  m_asr.program = sibling_binary("gladys-asr");
  m_asr.args = {model_dir};
  m_asr.url = api_url(ASR_PORT);
  m_llm.name = "gladys-llm";
  m_llm.program = llama_server_program();
  m_llm.args = llama_server_args();
  m_llm.url = api_url(LLM_PORT);
  m_llm.ready_path = "/health";  // 503 while loading, 200 ready
  m_llm.process.setProcessEnvironment(llama_server_env());

  for (Child* child : {&m_asr, &m_llm}) {
    connect(&child->process, &QProcess::readyReadStandardError, this, [this, child]() { read_stderr(*child); });
    connect(&child->process, &QProcess::finished, this, [this, child]() { on_exit(*child); });
  }

  m_probe_timer.setInterval(PROBE_INTERVAL_MS);
  connect(&m_probe_timer, &QTimer::timeout, this, [this]() {
    if (!m_asr.ready) {
      probe(m_asr);
    }
    if (!m_llm.ready) {
      probe(m_llm);
    }
  });
}

// Closing stdin quits gladys-asr; llama-server ignores stdin, so it is terminated. Kill if still running after the timeout.
Core::~Core() {
  m_stopping = true;
  m_asr.process.closeWriteChannel();
  m_llm.process.terminate();
  for (Child* child : {&m_asr, &m_llm}) {
    if (child->process.state() == QProcess::NotRunning) {
      continue;
    }
    if (!child->process.waitForFinished(STOP_TIMEOUT_MS)) {
      child->process.kill();
      child->process.waitForFinished(STOP_TIMEOUT_MS);
    }
  }
}

bool Core::start() {
  for (Child* child : {&m_asr, &m_llm}) {
    if (!QFileInfo::exists(child->program)) {
      qCCritical(lcMain) << child->name << "not found at:" << child->program;
      return false;
    }
    spawn(*child);
  }
  return true;
}

void Core::spawn(Child& child) {
  child.ready = false;
  child.process.start(child.program, child.args);
  m_probe_timer.start();
  qCInfo(lcMain) << child.name << "spawned";
}

// A child that dies before ready is fatal. One that dies later is restarted.
void Core::on_exit(Child& child) {
  if (m_stopping) {
    return;
  }
  qCWarning(lcMain) << child.name << "exited";
  if (!child.ready) {
    emit FAILED(child.name + " failed to start");
    return;
  }
  child.ready = false;

  if (&child == &m_asr && m_recording) {
    m_recording = false;
    emit RECORDING_CHANGED(false);
  }
  QTimer::singleShot(RESTART_DELAY_MS, this, [this, &child]() {
    if (!m_stopping) {
      spawn(child);
    }
  });
}

// Stderr arrives in chunks: complete lines are echoed to the console and stored with their level.
void Core::read_stderr(Child& child) {
  child.err_buf += child.process.readAllStandardError();
  for (int end = child.err_buf.indexOf('\n'); end >= 0; end = child.err_buf.indexOf('\n')) {
    const QString line = QString::fromUtf8(child.err_buf.left(end)).remove(ANSI_COLOR).trimmed();
    child.err_buf.remove(0, end + 1);
    if (line.isEmpty()) {
      continue;
    }
    std::fprintf(stderr, "%s\n", qPrintable(line));
    Data::post_log("[" + child.name + "] " + line, SEVERE_LEVEL.match(line).hasMatch(),
                   WARN_LEVEL.match(line).hasMatch());
  }
}

// 200 means ready. 503 (model loading) and connection errors retry on the next tick.
void Core::probe(Child& child) {
  QNetworkReply* reply = m_network.get(QNetworkRequest(QUrl(child.url + child.ready_path)));
  connect(reply, &QNetworkReply::finished, this, [this, reply, &child]() {
    reply->deleteLater();
    if (child.ready || reply->error() != QNetworkReply::NoError) {
      return;
    }
    child.ready = true;
    qCInfo(lcMain) << child.name << "ready";
    check_ready();
  });
}

void Core::check_ready() {
  if (!m_asr.ready || !m_llm.ready) {
    return;
  }
  m_probe_timer.stop();
  emit READY();
}

// POST JSON to an API. done gets the reply body, or an empty object on any failure.
void Core::post(const QString& url, const QJsonObject& body, std::function<void(const QJsonObject&)> done) {
  QNetworkRequest request{QUrl(url)};
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  request.setTransferTimeout(REQUEST_TIMEOUT_MS);
  QNetworkReply* reply = m_network.post(request, QJsonDocument(body).toJson(QJsonDocument::Compact));
  connect(reply, &QNetworkReply::finished, this, [reply, url, done]() {
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
      qCWarning(lcMain) << url << "failed:" << reply->errorString();
      done(QJsonObject());
      return;
    }
    done(QJsonDocument::fromJson(reply->readAll()).object());
  });
}

// Hotkey down. Toggle: each press flips recording. Hold: the press starts it.
void Core::hotkey_down() {
  if (!m_recording) {
    start_recording();
  } else if (!m_hold_mode) {
    stop_recording();
  }
}

// Hotkey up. Only Hold stops on release.
void Core::hotkey_up() {
  if (m_hold_mode && m_recording) {
    stop_recording();
  }
}

void Core::start_recording() {
  if (!m_asr.ready) {
    qCWarning(lcMain) << "gladys-asr not ready";
    return;
  }
  m_recording = true;
  emit RECORDING_CHANGED(true);
  const QJsonObject body{{"action", "start"},
                         {"inputSource", m_config.value("inputSource", "default").toString()}};
  post(m_asr.url + "/transcribe", body, [this](const QJsonObject& reply) {
    if (reply.isEmpty()) {
      m_recording = false;
      emit RECORDING_CHANGED(false);
    }
  });
}

void Core::stop_recording() {
  m_recording = false;
  emit RECORDING_CHANGED(false);
  add_pending(1);
  post(m_asr.url + "/transcribe", QJsonObject{{"action", "stop"}}, [this](const QJsonObject& reply) {
    format_text(reply.value("text").toString());
  });
}

void Core::update_config(const QVariantMap& config) {
  m_config = config;
  m_hold_mode = config.value("hotkeyMode").toString() == "Hold";
}

void Core::add_pending(int delta) {
  m_pending += delta;
  emit PROCESSING_CHANGED(m_pending > 0);
}

// Formats through llama-server. Any failure, or LLM disabled, pastes the raw text, so dictation is never lost.
void Core::format_text(const QString& text) {
  if (text.isEmpty()) {
    add_pending(-1);
    return;
  }
  if (!m_llm.ready || !m_config.value("llm", true).toBool()) {
    paste_text(text);
    return;
  }
  post(m_llm.url + "/v1/chat/completions", build_format_request(text, m_config),
       [this, text](const QJsonObject& reply) {
         paste_text(read_format_reply(reply, text, m_config.value("snippets").toMap()));
       });
}

// Clipboard is written on the GUI thread; the keystroke runs on the keypaste thread.
void Core::paste_text(const QString& text) {
  Data::post_transcription(text);
  QTimer::singleShot(LLM_TO_COPY_DELAY_MS, this, [this, text]() {
    Paste::copy_to_clipboard(text);
    QTimer::singleShot(COPY_TO_PASTE_DELAY_MS, m_paste, [this, paste = m_paste]() {
      paste->send_paste();
      QMetaObject::invokeMethod(this, [this]() { add_pending(-1); });  // back to the GUI thread
    });
  });
}
