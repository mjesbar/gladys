#include "gladysd.h"
#include "keygrab.h"
#include "keytype.h"
#include "llm.h"
#include "stt.h"
#include <QCoreApplication>
#include <QElapsedTimer>
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
      m_display(nullptr), m_sttThread(nullptr), m_stt(nullptr),
      m_keyTypeThread(nullptr), m_llm(nullptr), m_keyType(nullptr) {

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

  std::string exe_path = QCoreApplication::applicationDirPath().toStdString();
  std::string model_path =
      exe_path + "/models/sherpa-onnx-streaming-zipformer-es-kroko-2025-08-06";
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

  // Use QTimer instead of QSocketNotifier for more predictable CPU usage
  // X11 socket polling at 30fps max - hotkey detection doesn't need high
  // frequency
  QTimer *x11PollTimer = new QTimer(this);
  x11PollTimer->setInterval(33); // ~30fps
  connect(x11PollTimer, &QTimer::timeout, m_keyGrab, &KeyGrab::processEvents);
  x11PollTimer->start();

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
      // Toggle OFF: stop STT, flush, process with LLM
      STT::stop();
      STT::setBuffering(false);
      stt_running = false;

      // Get buffered audio and process with LLM
      QByteArray audioData = STT::getAudioBuffer();
      fprintf(stderr, "Gladysd: Audio buffer size: %d bytes\n",
              audioData.size());
      if (!audioData.isEmpty()) {
        fprintf(stderr, "Gladysd: Sending request to LLM...\n");
        QString result = LLM::transcriptFromMemory(audioData);
        STT::clearAudioBuffer();

        if (!result.isEmpty()) {
          fprintf(stderr, "Gladysd: LLM result: %s\n", qPrintable(result));
          // Copy to clipboard, select all, and paste
          m_keyType->copyToClipboard(result);
          usleep(100000); // 100ms for clipboard to be ready
          m_keyType->selectAllAndPaste();
        } else {
          fprintf(stderr, "Gladysd: LLM returned empty result\n");
        }
      } else {
        fprintf(stderr, "Gladysd: No audio data to transcribe\n");
      }

      // Reset keytype queue for clean state
      m_keyType->reset();
    } else {
      // Toggle ON: start STT and enable buffering
      STT::clearAudioBuffer();
      STT::setBuffering(true);
      STT::start();
      stt_running = true;
      fprintf(stderr, "Gladysd: STT started with audio buffering enabled\n");
    }
    emit toggleRequested();
  });

  connect(m_stt, &STT::audioLevelUpdated, this, &Gladysd::audioLevelUpdated);

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
