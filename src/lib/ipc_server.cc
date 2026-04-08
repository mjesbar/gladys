#include "ipc_server.h"
#include <QDebug>

IpcServer::IpcServer(const QString &serverName, QObject *parent)
    : QObject(parent), m_server(new QLocalServer(this)), m_serverName(serverName) {
}

IpcServer::~IpcServer() = default;

bool IpcServer::start() {
  QLocalServer::removeServer(m_serverName);
  if (!m_server->listen(m_serverName)) {
    qWarning() << "IpcServer: Could not start server:" << m_server->errorString();
    return false;
  }
  qDebug() << "IpcServer: Listening on" << m_serverName;
  connect(m_server, &QLocalServer::newConnection, this, &IpcServer::onNewConnection);
  return true;
}

void IpcServer::onNewConnection() {
  qDebug() << "IpcServer: New connection";
  QLocalSocket *clientConnection = m_server->nextPendingConnection();
  connect(clientConnection, &QLocalSocket::readyRead,
          [this, clientConnection]() {
            QByteArray data = clientConnection->readAll();
            qDebug() << "IpcServer: Received:" << data;
            if (data == "toggle") {
              emit toggleRequested();
            }
            clientConnection->disconnectFromServer();
          });
  connect(clientConnection, &QLocalSocket::disconnected,
          clientConnection, &QLocalSocket::deleteLater);
}

IpcClient::IpcClient(const QString &serverName, QObject *parent)
    : QObject(parent), m_serverName(serverName) {
}

bool IpcClient::sendToggle() {
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