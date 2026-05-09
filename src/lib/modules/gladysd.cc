#include "gladysd.h"
#include "stt.h"
#include "llm.h"
#include "keytype.h"
#include "keygrab.h"
#include <X11/Xlib.h>
#include <csignal>
#include <cstdio>
#include <unistd.h>

Gladysd *Gladysd::s_instance = nullptr;

Gladysd *Gladysd::instance() {
  if (!s_instance) {
    s_instance = new Gladysd();
  }
  return s_instance;
}

Gladysd::Gladysd(QObject *parent) : QObject(parent), m_ydotoolPid(0), m_ydotoolProcess(nullptr), m_ydotoolTimer(new QTimer(this)), m_ipcServer(nullptr), m_ipcServerName("gladys-ipc-server"), m_keyGrab(nullptr), m_display(nullptr), m_x11Notifier(nullptr), m_sttThread(nullptr), m_stt(nullptr), m_llm(nullptr), m_keyType(nullptr) {

  connect(m_ydotoolTimer, &QTimer::timeout, this, [this]() {
    if (m_ydotoolPid > 0 && kill(m_ydotoolPid, 0) != 0) {
      m_ydotoolTimer->stop();
      fprintf(stderr, "Gladysd: ydotoold exited.\n");
      emit ydotoolExited();
    }
  });

  std::signal(SIGTERM, signalHandler);
  std::signal(SIGINT, signalHandler);
}

Gladysd::~Gladysd() {
  shutdown();
}

void Gladysd::signalHandler(int signum) {
  if (signum == SIGTERM || signum == SIGINT) {
    fprintf(stderr, "Gladysd: Received signal %d, shutting down.\n", signum);
    instance()->shutdown();
    _exit(0);
  }
}

bool Gladysd::init() {
  m_ipcServer = new IPCServer(m_ipcServerName, this);
  if (!m_ipcServer->start()) {
    fprintf(stderr, "Gladysd: IPC server failed to start.\n");
    return false;
  }

  std::string model_path =
      "./bin/models/sherpa-onnx-streaming-zipformer-es-kroko-2025-08-06";
  if (!STT::load(model_path)) {
    fprintf(stderr, "Gladysd: Failed to load STT model.\n");
    return false;
  }

  m_llm = LLM::instance();
  if (!LLM::start()) {
    fprintf(stderr, "Gladysd: Failed to start LLM server.\n");
    return false;
  }

  m_display = static_cast<void*>(XOpenDisplay(NULL));
  if (!m_display) {
    fprintf(stderr, "Gladysd: Unable to open X display\n");
    return false;
  }

  m_keyGrab = new KeyGrab(this);
  if (!m_keyGrab->init(m_display)) {
    return false;
  }

  int x11_fd = ConnectionNumber(static_cast<Display*>(m_display));
  m_x11Notifier = new QSocketNotifier(x11_fd, QSocketNotifier::Read, this);
  connect(m_x11Notifier, &QSocketNotifier::activated, [this]() {
    m_keyGrab->processEvents();
  });

  m_stt = STT::instance();
  m_sttThread = new QThread();
  m_stt->moveToThread(m_sttThread);
  m_sttThread->start();

  launchYdotool();
  setupConnections();

  fprintf(stderr, "Gladysd: Initialized successfully.\n");
  return true;
}

void Gladysd::launchProcess(const QString &program, qint64 &pid) {
  QProcess *proc = new QProcess();
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert("QT_QPA_PLATFORM", "xcb");

  if (program.contains("ydotoold")) {
    m_ydotoolProcess = proc;
  }

  proc->setProcessEnvironment(env);
  proc->setProgram(program);
  proc->start();
  pid = proc->processId();
  fprintf(stderr, "Gladysd: launched %s (PID %lld).\n",
          qPrintable(program), (long long)pid);
}

void Gladysd::killProcess(qint64 pid) {
  if (pid > 0 && (kill(pid, 0) == 0 || errno == EPERM)) {
    kill(pid, SIGTERM);
    fprintf(stderr, "Gladysd: killed PID %lld.\n", (long long)pid);
  }
}

void Gladysd::launchYdotool() {
  launchProcess("bin/ydotool/ydotoold", m_ydotoolPid);
  m_ydotoolTimer->start(1000);
}

void Gladysd::setupConnections() {
  connect(m_keyGrab, &KeyGrab::keyPressed, this, [this]() {
    fprintf(stderr, "Gladysd: Ctrl+Alt+P detected!\n");
    static bool stt_running = false;
    if (stt_running) {
      STT::stop();
      stt_running = false;
    } else {
      STT::start();
      stt_running = true;
    }
    emit toggleRequested();
  });

  connect(m_stt, &STT::audioLevelUpdated, this, [this]() {
    emit audioLevelUpdated(STT::getAudioLevels());
  });

  connect(m_stt, &STT::textReceived, this, [](const QString &text) {
    KeyType::instance()->push(text);
  });
}

void Gladysd::shutdown() {
  fprintf(stderr, "Gladysd: Shutting down...\n");

  LLM::stop();

  if (m_sttThread) {
    m_sttThread->quit();
    m_sttThread->wait();
  }

  if (m_ipcServer) {
    m_ipcServer->deleteLater();
    m_ipcServer = nullptr;
  }

  if (m_ydotoolPid > 0) {
    killProcess(m_ydotoolPid);
    m_ydotoolTimer->stop();
  }

  if (m_display) {
    XCloseDisplay(static_cast<Display*>(m_display));
    m_display = nullptr;
  }

  fprintf(stderr, "Gladysd: Shutdown complete.\n");
}