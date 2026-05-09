// IPC Module - Local socket-based inter-process communication.

#ifndef IPC_H
#define IPC_H

#include <QObject>
#include <QLocalServer>
#include <QLocalSocket>
#include <QString>
#include <QVector>

class IPCServer : public QObject {
  Q_OBJECT

public:
  explicit IPCServer(const QString &serverName, QObject *parent = nullptr);
  ~IPCServer() override;

  bool start();

signals:
  void toggleRequested();
  void quitRequested();
  void audioLevelUpdated(const QVector<double> &levels);

private:
  void onNewConnection();

  QLocalServer *m_server;
  QString m_serverName;
};

class IPCClient : public QObject {
  Q_OBJECT

public:
  explicit IPCClient(const QString &serverName, QObject *parent = nullptr);

  bool sendToggle();
  bool sendQuit();
  bool sendAudioLevels(const QVector<double> &levels);

private:
  QString m_serverName;
};

#endif // IPC_H