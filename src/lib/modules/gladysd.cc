#include "gladysd.h"
#include "keygrab.h"
#include "keytype.h"
#include "llm.h"
#include "stt.h"
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

Gladysd::Gladysd(QObject *parent)
    : QObject(parent), m_ipcServer(nullptr),
      m_ipcServerName("gladys-ipc-server"), m_keyGrab(nullptr),
      m_display(nullptr), m_x11Notifier(nullptr), m_sttThread(nullptr),
      m_stt(nullptr), m_keyTypeThread(nullptr), m_llm(nullptr),
      m_keyType(nullptr) {

  std::signal(SIGTERM, signalHandler);
  std::signal(SIGINT, signalHandler);
}

Gladysd::~Gladysd() { shutdown(); }

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

  m_display = static_cast<void *>(XOpenDisplay(NULL));
  if (!m_display) {
    fprintf(stderr, "Gladysd: Unable to open X display\n");
    return false;
  }

  m_keyGrab = new KeyGrab(this);
  if (!m_keyGrab->init(m_display)) {
    return false;
  }

  int x11_fd = ConnectionNumber(static_cast<Display *>(m_display));
  m_x11Notifier = new QSocketNotifier(x11_fd, QSocketNotifier::Read, this);
  connect(m_x11Notifier, &QSocketNotifier::activated,
          [this]() { m_keyGrab->processEvents(); });

  m_stt = STT::instance();
  m_sttThread = new QThread();
  m_stt->moveToThread(m_sttThread);
  m_sttThread->start();

  // KeyType runs on its own thread to avoid blocking main thread
  m_keyType = KeyType::instance();
  m_keyTypeThread = new QThread();
  m_keyType->moveToThread(m_keyTypeThread);
  m_keyTypeThread->start();

  setupConnections();

  fprintf(stderr, "Gladysd: Initialized successfully.\n");
  return true;
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

  // Throttle audio levels to ~30fps to reduce UI load
  static QElapsedTimer throttleTimer;
  static bool initialized = false;
  connect(m_stt, &STT::audioLevelUpdated, this,
          [this]() { emit audioLevelUpdated(STT::getAudioLevels()); });

  connect(m_stt, &STT::textReceived, m_keyType, &KeyType::push);
}

void Gladysd::shutdown() {
  fprintf(stderr, "Gladysd: Shutting down...\n");

  LLM::stop();

  if (m_sttThread) {
    m_sttThread->quit();
    m_sttThread->wait();
  }

  if (m_keyTypeThread) {
    m_keyTypeThread->quit();
    m_keyTypeThread->wait();
  }

  if (m_ipcServer) {
    m_ipcServer->deleteLater();
    m_ipcServer = nullptr;
  }

  if (m_display) {
    XCloseDisplay(static_cast<Display *>(m_display));
    m_display = nullptr;
  }

  fprintf(stderr, "Gladysd: Shutdown complete.\n");
}
