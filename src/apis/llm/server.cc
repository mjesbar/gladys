// gladys-llm - Transcription formatting API: GET /alive, POST /format on 127.0.0.1:8082.
// Owns llama-server. Exits when its stdin closes, which stops llama-server too.

#include <QCoreApplication>
#include <QHttpServer>
#include <QTcpServer>

#include <memory>

#include "../../modules/log/log.hpp"
#include "../server.hpp"
#include "llm.hpp"

int main(int argc, char* argv[]) {
  log_setup();
  QCoreApplication app(argc, argv);

  LLM llm;  // destroyed after exec returns, which stops llama-server
  if (!llm.start()) {
    return 1;
  }

  QHttpServer server;
  server.route("/alive", QHttpServerRequest::Method::Get, [&llm]() {
    if (!llm.is_ready()) {
      return error_reply(QHttpServerResponder::StatusCode::ServiceUnavailable, "model loading");
    }
    return QHttpServerResponse(QJsonObject{{"status", "ok"}});
  });

  // Replies later, once llama-server answers; the responder must outlive the handler.
  server.route("/format", QHttpServerRequest::Method::Post,
               [&llm](const QHttpServerRequest& request, QHttpServerResponder& responder) {
    const QJsonObject body = parse_body(request);
    auto reply = std::make_shared<QHttpServerResponder>(std::move(responder));
    if (body.isEmpty()) {
      reply->sendResponse(error_reply(QHttpServerResponder::StatusCode::BadRequest, "invalid JSON body"));
      return;
    }
    if (!llm.is_ready()) {
      reply->sendResponse(error_reply(QHttpServerResponder::StatusCode::ServiceUnavailable, "model loading"));
      return;
    }
    llm.format(body.value("text").toString(), body.toVariantMap(), [reply](const QString& text) {
      reply->sendResponse(QHttpServerResponse(QJsonObject{{"text", text}}));
    });
  });

  QTcpServer tcp;
  if (!serve_local(server, tcp, LLM_PORT)) {
    qCCritical(lcLlm) << "failed to listen on port" << LLM_PORT;
    return 1;
  }
  qCInfo(lcLlm) << "listening on 127.0.0.1:" << LLM_PORT;

  quit_when_stdin_closes();
  return app.exec();
}
