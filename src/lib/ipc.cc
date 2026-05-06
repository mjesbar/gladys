#include "ipc.h"
#include <QDebug>

IPCServer::IPCServer(const QString &serverName, QObject *parent)
    : QObject(parent), m_server(new QLocalServer(this)), m_serverName(serverName) {
}

IPCServer::~IPCServer() = default;

bool IPCServer::start() {
  QLocalServer::removeServer(m_serverName);
  if (!m_server->listen(m_serverName)) {
    qWarning() << "IPCServer: Could not start server:" << m_server->errorString();
    return false;
  }
  qDebug() << "IPCServer: Listening on" << m_serverName;
  connect(m_server, &QLocalServer::newConnection, this, &IPCServer::onNewConnection);
  return true;
}

void IPCServer::onNewConnection() {
  qDebug() << "IPCServer: New connection";
  QLocalSocket *clientConnection = m_server->nextPendingConnection();
  connect(clientConnection, &QLocalSocket::readyRead,
          [this, clientConnection]() {
            QByteArray data = clientConnection->readAll();
            qDebug() << "IPCServer: Received:" << data;
            if (data == "toggle") {
              emit toggleRequested();
            } else if (data == "quit") {
              emit quitRequested();
            }
            clientConnection->disconnectFromServer();
          });
  connect(clientConnection, &QLocalSocket::disconnected,
          clientConnection, &QLocalSocket::deleteLater);
}

IPCClient::IPCClient(const QString &serverName, QObject *parent)
    : QObject(parent), m_serverName(serverName) {
}

bool IPCClient::sendToggle() {
  QLocalSocket socket;
  socket.connectToServer(m_serverName);
  if (!socket.waitForConnected(200)) {
    return false;
  }
  socket.write("toggle");
  socket.waitForBytesWritten(1000);
  socket.disconnectFromServer();
  return true;
}

bool IPCClient::sendQuit() {
  QLocalSocket socket;
  socket.connectToServer(m_serverName);
  if (!socket.waitForConnected(200)) {
    return false;
  }
  socket.write("quit");
  socket.waitForBytesWritten(1000);
  socket.disconnectFromServer();
  return true;
}