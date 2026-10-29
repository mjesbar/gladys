// gladys-asr - Speech-to-text API: GET /alive, POST /transcribe on 127.0.0.1:8081.
// Usage: gladys-asr <model_dir>. Exits when its stdin closes.

#include <QCoreApplication>
#include <QHttpServer>
#include <QTcpServer>

#include "../../modules/log/log.hpp"
#include "../server.hpp"
#include "stt.hpp"

// {"action":"start","inputSource":"..."} starts capture; {"action":"stop"} returns {"text":"..."}.
static QHttpServerResponse transcribe(STT& stt, const QHttpServerRequest& request) {
  const QJsonObject body = parse_body(request);
  const QString action = body.value("action").toString();
  if (action == "start") {
    if (!stt.start_recording(body.value("inputSource").toString(QStringLiteral("default")))) {
      return error_reply(QHttpServerResponder::StatusCode::ServiceUnavailable, "capture unavailable");
    }
    return QJsonObject{{"status", "recording"}};
  }
  if (action == "stop") {
    return QJsonObject{{"text", stt.stop_recording()}};
  }
  return error_reply(QHttpServerResponder::StatusCode::BadRequest, "action must be start or stop");
}

int main(int argc, char* argv[]) {
  log_setup();
  QCoreApplication app(argc, argv);
  if (argc < 2) {
    qCCritical(lcStt) << "usage: gladys-asr <model_dir>";
    return 1;
  }

  STT stt;
  if (!stt.init(argv[1])) {
    return 1;
  }

  QHttpServer server;
  server.route("/alive", QHttpServerRequest::Method::Get,
               []() { return QHttpServerResponse(QJsonObject{{"status", "ok"}}); });
  server.route("/transcribe", QHttpServerRequest::Method::Post,
               [&stt](const QHttpServerRequest& request) { return transcribe(stt, request); });

  QTcpServer tcp;
  if (!serve_local(server, tcp, ASR_PORT)) {
    qCCritical(lcStt) << "failed to listen on port" << ASR_PORT;
    return 1;
  }
  qCInfo(lcStt) << "listening on 127.0.0.1:" << ASR_PORT;

  quit_when_stdin_closes();
  return app.exec();
}
