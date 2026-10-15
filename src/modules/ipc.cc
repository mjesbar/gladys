#include "ipc.hpp"
#include <QDebug>

IPCServer::IPCServer(const QString &serverName, QObject *parent)
    : QObject(parent), m_server(new QLocalServer(this)), m_server_name(serverName) {
}

IPCServer::~IPCServer() = default;

bool IPCServer::start() {
  QLocalServer::removeServer(m_server_name);
  if (!m_server->listen(m_server_name)) {
    qWarning() << "IPCServer: Could not start server:" << m_server->errorString();
    return false;
  }
  qDebug() << "IPCServer: Listening on" << m_server_name;
  connect(m_server, &QLocalServer::newConnection, this, &IPCServer::on_new_connection);
  return true;
}

void IPCServer::on_new_connection() {
  qDebug() << "IPCServer: New connection";
  QLocalSocket *clientConnection = m_server->nextPendingConnection();
  connect(clientConnection, &QLocalSocket::readyRead,
          [this, clientConnection]() {
            QByteArray data = clientConnection->readAll();
            qDebug() << "IPCServer: Received:" << data;
            if (data == "toggle") {
              emit toggle_requested();
            } else if (data == "quit") {
              emit quit_requested();
            } else if (data.startsWith("levels:")) {
              QStringList parts = QString::fromUtf8(data).split(':');
              if (parts.size() == 2) {
                QVector<double> levels;
                for (const QString &s : parts[1].split(',')) {
                  levels.append(s.toDouble());
                }
                emit audio_level_updated(levels);
              }
            }
            clientConnection->disconnectFromServer();
          });
  connect(clientConnection, &QLocalSocket::disconnected,
          clientConnection, &QLocalSocket::deleteLater);
}

IPCClient::IPCClient(const QString &serverName, QObject *parent)
    : QObject(parent), m_server_name(serverName) {
}

bool IPCClient::send_toggle() {
  QLocalSocket socket;
  socket.connectToServer(m_server_name);
  if (!socket.waitForConnected(200)) {
    return false;
  }
  socket.write("toggle");
  socket.waitForBytesWritten(1000);
  socket.disconnectFromServer();
  return true;
}

bool IPCClient::send_quit() {
  QLocalSocket socket;
  socket.connectToServer(m_server_name);
  if (!socket.waitForConnected(200)) {
    return false;
  }
  socket.write("quit");
  socket.waitForBytesWritten(1000);
  socket.disconnectFromServer();
  return true;
}

bool IPCClient::send_audio_levels(const QVector<double> &levels) {
  QLocalSocket socket;
  socket.connectToServer(m_server_name);
  if (!socket.waitForConnected(200)) {
    return false;
  }
  QStringList levelStrings;
  for (double level : levels) {
    levelStrings.append(QString::number(level));
  }
  QString msg = "levels:" + levelStrings.join(",");
  socket.write(msg.toUtf8());
  socket.waitForBytesWritten(1000);
  socket.disconnectFromServer();
  return true;
}