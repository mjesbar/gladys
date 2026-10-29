// Core - Pipeline and supervisor for gladys-asr and gladys-llm. Runs on the GUI thread.

#include "core.hpp"

#include <QCoreApplication>
#include <QFileInfo>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

#include "apis/server.hpp"
#include "modules/keypaste/paste.hpp"
#include "modules/log/log.hpp"

#ifdef Q_OS_WIN
#define EXE_SUFFIX ".exe"
#else
#define EXE_SUFFIX ""
#endif

static const int RESTART_DELAY_MS = 1000;
static const int STOP_TIMEOUT_MS = 6000;  // llm needs time to stop llama-server
static const int PROBE_INTERVAL_MS = 250;
static const int REQUEST_TIMEOUT_MS = 30000;  // a stop includes the transcription
static const int LLM_TO_COPY_DELAY_MS = 10;
static const int COPY_TO_PASTE_DELAY_MS = 250;

static QString sibling_binary(const char* name) {
  return QCoreApplication::applicationDirPath() + "/" + name + EXE_SUFFIX;
}

static QString api_url(quint16 port) { return QStringLiteral("http://127.0.0.1:%1").arg(port); }

Core::Core(const QString& model_dir, Paste* paste, QObject* parent)
    : QObject(parent), m_paste(paste) {
  m_asr.name = "gladys-asr";
  m_asr.program = sibling_binary("gladys-asr");
  m_asr.args = {model_dir};
  m_asr.url = api_url(ASR_PORT);
  m_llm.name = "gladys-llm";
  m_llm.program = sibling_binary("gladys-llm");
  m_llm.url = api_url(LLM_PORT);

  for (Child* child : {&m_asr, &m_llm}) {
    child->process.setProcessChannelMode(QProcess::ForwardedErrorChannel);
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

// Closing stdin tells each child to quit; kill only if it does not finish in time.
Core::~Core() {
  m_stopping = true;
  for (Child* child : {&m_asr, &m_llm}) {
    if (child->process.state() == QProcess::NotRunning) {
      continue;
    }
    child->process.closeWriteChannel();
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

// 200 on /alive means ready. 503 (model loading) and connection errors retry on the next tick.
void Core::probe(Child& child) {
  QNetworkReply* reply = m_network.get(QNetworkRequest(QUrl(child.url + "/alive")));
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

// Hotkey: the first press starts capture, the second stops it and formats the text.
void Core::toggle_recording() {
  if (!m_asr.ready) {
    qCWarning(lcMain) << "gladys-asr not ready";
    return;
  }
  m_recording = !m_recording;
  emit RECORDING_CHANGED(m_recording);

  if (m_recording) {
    const QJsonObject body{{"action", "start"},
                           {"inputSource", m_config.value("inputSource", "default").toString()}};
    post(m_asr.url + "/transcribe", body, [this](const QJsonObject& reply) {
      if (reply.isEmpty()) {
        m_recording = false;
        emit RECORDING_CHANGED(false);
      }
    });
  } else {
    add_pending(1);
    post(m_asr.url + "/transcribe", QJsonObject{{"action", "stop"}}, [this](const QJsonObject& reply) {
      format_text(reply.value("text").toString());
    });
  }
}

void Core::update_config(const QVariantMap& config) { m_config = config; }

void Core::add_pending(int delta) {
  m_pending += delta;
  emit PROCESSING_CHANGED(m_pending > 0);
}

// Formats through the LLM API. Any failure pastes the raw text, so dictation is never lost.
void Core::format_text(const QString& text) {
  if (text.isEmpty()) {
    add_pending(-1);
    return;
  }
  if (!m_llm.ready) {
    paste_text(text);
    return;
  }
  QJsonObject body = QJsonObject::fromVariantMap(m_config);
  body["text"] = text;
  post(m_llm.url + "/format", body, [this, text](const QJsonObject& reply) {
    const QString formatted = reply.value("text").toString();
    paste_text(formatted.isEmpty() ? text : formatted);
  });
}

// Clipboard is written on the GUI thread; the keystroke runs on the keypaste thread.
void Core::paste_text(const QString& text) {
  QTimer::singleShot(LLM_TO_COPY_DELAY_MS, this, [this, text]() {
    Paste::copy_to_clipboard(text);
    QTimer::singleShot(COPY_TO_PASTE_DELAY_MS, m_paste, [this, paste = m_paste]() {
      paste->send_paste();
      QMetaObject::invokeMethod(this, [this]() { add_pending(-1); });  // back to the GUI thread
    });
  });
}
