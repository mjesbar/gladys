// Server - Shared QHttpServer helpers for the API processes. Loopback only.

#ifndef SERVER_HPP
#define SERVER_HPP

#include <QCoreApplication>
#include <QHostAddress>
#include <QHttpServer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMetaObject>
#include <QTcpServer>
#include <QThread>

#include <iostream>
#include <string>

constexpr quint16 ASR_PORT = 8081;  // GET /alive, POST /transcribe
constexpr quint16 LLM_PORT = 8082;  // GET /alive, POST /format

static const qsizetype MAX_BODY_BYTES = 64 * 1024;

// Empty object when the body is not a JSON object or is too large.
inline QJsonObject parse_body(const QHttpServerRequest& request) {
  const QByteArray body = request.body();
  if (body.size() > MAX_BODY_BYTES) {
    return {};
  }
  const QJsonDocument doc = QJsonDocument::fromJson(body);
  return doc.isObject() ? doc.object() : QJsonObject();
}

inline QHttpServerResponse error_reply(QHttpServerResponder::StatusCode status, const QString& message) {
  return QHttpServerResponse(QJsonObject{{"error", message}}, status);
}

// Binds the server to 127.0.0.1 only: the APIs are never reachable from the network.
inline bool serve_local(QHttpServer& server, QTcpServer& tcp, quint16 port) {
  return tcp.listen(QHostAddress::LocalHost, port) && server.bind(&tcp);
}

// The parent (gladys) closes our stdin when it exits: quit with it.
inline void quit_when_stdin_closes() {
  QThread* reader = QThread::create([]() {
    std::string line;
    while (std::getline(std::cin, line)) {
    }
    QMetaObject::invokeMethod(qApp, &QCoreApplication::quit);
  });
  QObject::connect(reader, &QThread::finished, reader, &QObject::deleteLater);
  reader->start();
}

#endif  // SERVER_HPP
