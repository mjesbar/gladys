#include "pm.h"
#include <csignal>
#include <cstdio>
#include <unistd.h>

ProcessManager *ProcessManager::s_instance = nullptr;

ProcessManager *ProcessManager::instance() {
  if (!s_instance) {
    s_instance = new ProcessManager();
  }
  return s_instance;
}

ProcessManager::ProcessManager(QObject *parent)
    : QObject(parent), m_role(RoleDaemon), m_windowAppPid(0), m_daemonPid(0),
      m_ydotoolPid(0),
      m_windowAppProcess(nullptr), m_daemonProcess(nullptr),
      m_ydotoolProcess(nullptr),
      m_windowAppTimer(new QTimer(this)), m_daemonTimer(new QTimer(this)),
      m_ydotoolTimer(new QTimer(this)) {
  // Monitor window app
  connect(m_windowAppTimer, &QTimer::timeout, this, [this]() {
    if (m_windowAppPid > 0 && kill(m_windowAppPid, 0) != 0) {
      m_windowAppTimer->stop();
      fprintf(stderr, "ProcessManager: window app exited.\n");
      emit windowAppExited();
    }
  });

  // Monitor daemon
  connect(m_daemonTimer, &QTimer::timeout, this, [this]() {
    if (m_daemonPid > 0 && kill(m_daemonPid, 0) != 0) {
      m_daemonTimer->stop();
      fprintf(stderr, "ProcessManager: daemon exited.\n");
      emit daemonExited();
    }
  });

  // Monitor ydotoold
  connect(m_ydotoolTimer, &QTimer::timeout, this, [this]() {
    if (m_ydotoolPid > 0 && kill(m_ydotoolPid, 0) != 0) {
      m_ydotoolTimer->stop();
      fprintf(stderr, "ProcessManager: ydotoold exited.\n");
      emit ydotoolExited();
    }
  });
}

ProcessManager::~ProcessManager() = default;

void ProcessManager::init(Role role) {
  m_role = role;

  // Install signal handler only for daemon role (daemon receives Ctrl+C)
  if (role == RoleDaemon) {
    std::signal(SIGTERM, signalHandler);
    std::signal(SIGINT, signalHandler);
  }
}

void ProcessManager::signalHandler(int signum) {
  if (signum == SIGTERM || signum == SIGINT) {
    fprintf(stderr, "ProcessManager: Received signal %d, shutting down.\n", signum);
    ProcessManager *pm = instance();
    pm->close();

    // Exit this process
    if (pm->m_role == RoleDaemon) {
      // If daemon, terminate window app first
      if (pm->m_windowAppPid > 0) {
        kill(pm->m_windowAppPid, SIGTERM);
      }
    } else {
      // If window app, terminate daemon first
      if (pm->m_daemonPid > 0) {
        kill(pm->m_daemonPid, SIGTERM);
      }
    }

    // Use _exit for immediate exit in signal context
    _exit(0);
  }
}

qint64 ProcessManager::otherPid() const {
  return (m_role == RoleDaemon) ? m_windowAppPid : m_daemonPid;
}

void ProcessManager::launchProcess(const QString &program, qint64 &pid, qint64 otherPid) {
  QProcess *proc = new QProcess();
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert("QT_QPA_PLATFORM", "xcb");

  // Pass other process PID via environment
  if (otherPid > 0) {
    if (program.contains("gladys") && !program.contains("d")) {
      env.insert("GLADYSD_PID", QString::number(otherPid));
    } else {
      env.insert("GLADYS_PID", QString::number(otherPid));
    }
  }

  proc->setProcessEnvironment(env);
  proc->setProgram(program);

  if (program.contains("gladys") && !program.contains("d")) {
    m_windowAppProcess = proc;
  } else if (program.contains("ydotoold")) {
    m_ydotoolProcess = proc;
  } else {
    m_daemonProcess = proc;
  }

  proc->start();
  pid = proc->processId();
  fprintf(stderr, "ProcessManager: launched %s (PID %lld).\n",
          qPrintable(program), (long long)pid);
}

void ProcessManager::launchWindowApp() {
  launchProcess("bin/gladys", m_windowAppPid, m_daemonPid);
  m_windowAppTimer->start(1000);
}

void ProcessManager::launchDaemon() {
  launchProcess("bin/gladysd", m_daemonPid, m_windowAppPid);
  m_daemonTimer->start(1000);
}

void ProcessManager::launchYdotool() {
  launchProcess("bin/ydotool/ydotoold", m_ydotoolPid, 0);
  m_ydotoolTimer->start(1000);
}

void ProcessManager::killProcess(qint64 pid) {
  if (pid > 0 && (kill(pid, 0) == 0 || errno == EPERM)) {
    kill(pid, SIGTERM);
    fprintf(stderr, "ProcessManager: killed PID %lld.\n", (long long)pid);
  }
}

void ProcessManager::close() {
  fprintf(stderr, "ProcessManager: closing all processes.\n");
  killProcess(m_windowAppPid);
  killProcess(m_daemonPid);
  killProcess(m_ydotoolPid);
  m_windowAppTimer->stop();
  m_daemonTimer->stop();
  m_ydotoolTimer->stop();
}