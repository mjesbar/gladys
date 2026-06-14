// Gladysd Hub - Main controller managing all system components.

#include "gladysd.h"
#include "keygrab.h"
#include "keytype.h"
#include "llm.h"
#include "stt.h"
#include "settings/settings_manager.h"
#include <QCoreApplication>
#include <QElapsedTimer>
#include <csignal>
#include <cstdio>

// Cross-platform sleep (milliseconds)
#ifdef _WIN32
#define SleepMs(ms) Sleep(ms)
#else
#include <unistd.h>
#define SleepMs(ms) usleep((ms) * 1000)
#endif

#ifdef __linux__
#include <X11/Xlib.h>
#endif

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
      exe_path +
      "/sherpaonnx/sherpa-onnx-streaming-zipformer-es-kroko-2025-08-06";
  if (!STT::load(model_path)) {
    fprintf(stderr, "Gladysd: Failed to load STT model.\n");
    return false;
  }

  m_llm = LLM::instance();
  if (!LLM::start()) {
    fprintf(stderr, "Gladysd: Failed to start LLM server.\n");
    return false;
  }

#ifdef __linux__
  m_display = static_cast<void *>(XOpenDisplay(NULL));
  if (!m_display) {
    fprintf(stderr, "Gladysd: Unable to open X display\n");
    return false;
  }
#else
  m_display = nullptr;
#endif

  m_keyGrab = new KeyGrab(this);
  if (!m_keyGrab->init(m_display)) {
    return false;
  }

#ifdef __linux__
  // Use QTimer instead of QSocketNotifier for more predictable CPU usage
  // X11 socket polling at 30fps max - hotkey detection doesn't need high
  // frequency
  QTimer *x11PollTimer = new QTimer(this);
  x11PollTimer->setInterval(33); // ~30fps
  connect(x11PollTimer, &QTimer::timeout, m_keyGrab, &KeyGrab::processEvents);
  x11PollTimer->start();
#endif

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
      // Toggle OFF: stop STT
      STT::stop();
      stt_running = false;
    } else {
      // Toggle ON: start STT
      STT::start();
      stt_running = true;
      fprintf(stderr, "Gladysd: STT started\n");
    }
    emit toggleRequested();
  });

  connect(m_stt, &STT::finalTextReady, this, [this](const QString &sttText) {
    fprintf(stderr, "Gladysd: STT text: %s\n", qPrintable(sttText));

    if (!sttText.isEmpty()) {
      fprintf(stderr, "Gladysd: Sending to LLM for transcription formatting...\n");
      QString result = LLM::formatTranscription(sttText);

      if (!result.isEmpty()) {
        fprintf(stderr, "Gladysd: LLM result: %s\n", qPrintable(result));
        // Copy to clipboard, select all, and paste
        m_keyType->copyToClipboard(result);
        SleepMs(100); // 100ms for clipboard to be ready
        m_keyType->selectAllAndPaste();
      } else {
        fprintf(stderr, "Gladysd: LLM returned empty result\n");
      }
    } else {
      fprintf(stderr, "Gladysd: No STT text to beautify\n");
    }
  });

  connect(m_stt, &STT::audioLevelUpdated, this, &Gladysd::audioLevelUpdated);

  // React to settings changes (input source, style, dictionary)
  connect(SettingsManager::instance(), &SettingsManager::settingsChanged, this,
          [this]() {
            fprintf(stderr, "Gladysd: Settings changed, reconfiguring STT audio source.\n");
            STT::reconfigureAudioSource();
          });
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

#ifdef __linux__
  if (m_display) {
    XCloseDisplay(static_cast<Display *>(m_display));
    m_display = nullptr;
  }
#endif

  fprintf(stderr, "Gladysd: Shutdown complete.\n");
}
