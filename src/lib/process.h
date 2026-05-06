#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <QObject>
#include <QProcess>
#include <QTimer>

class ProcessManager : public QObject {
  Q_OBJECT

public:
  static ProcessManager *instance();

  enum Role { RoleDaemon, RoleWindowApp };

  void init(Role role);
  Role role() const { return m_role; }

  void launchWindowApp();
  void launchDaemon();

  qint64 windowAppPid() const { return m_windowAppPid; }
  qint64 daemonPid() const { return m_daemonPid; }
  qint64 otherPid() const;

  void close();

signals:
  void windowAppExited();
  void daemonExited();

private:
  static ProcessManager *s_instance;

  explicit ProcessManager(QObject *parent = nullptr);
  ~ProcessManager() override;

  static void signalHandler(int signum);

  void launchProcess(const QString &program, qint64 &pid, qint64 otherPid);
  void killProcess(qint64 pid);

  Role m_role;
  qint64 m_windowAppPid;
  qint64 m_daemonPid;
  QProcess *m_windowAppProcess;
  QProcess *m_daemonProcess;
  QTimer *m_windowAppTimer;
  QTimer *m_daemonTimer;
};

#endif // PROCESS_HPP