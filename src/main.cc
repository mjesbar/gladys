// Gladys - Entry point. Creates the modules, wires their signals and owns
// shutdown.

#include <QApplication>
#include <QEventLoop>
#include <QFont>
#include <QFontDatabase>
#include <QIcon>
#include <QMetaObject>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSocketNotifier>
#include <QThread>
#include <QTimer>
#include <QWindow>

#include "modules/ai/llm.hpp"
#include "modules/ai/stt.hpp"
#include "modules/key/paste.hpp"
#include "modules/key/toggle.hpp"
#include "modules/log/log.hpp"
#include "modules/setting/setting.hpp"
#include "modules/ui/audio_bars.hpp"
#include "modules/ui/settings.hpp"
#include "modules/ui/tray.hpp"
#include "modules/ui/x11_shadow.hpp"

constexpr int STT_TO_LLM_DELAY_MS = 10;
constexpr int LLM_TO_COPY_DELAY_MS = 10;
constexpr int COPY_TO_PASTE_DELAY_MS = 250;

#ifndef _WIN32
#include <unistd.h>

#include <csignal>

static int g_signal_pipe[2];

// Async-signal-safe: only writes a byte; the Qt loop does the real shutdown.
static void on_signal(int) {
  char c = 0;
  (void)!write(g_signal_pipe[1], &c, 1);
}
#endif

#ifdef _WIN32
#include <windows.h>

#include <cstdio>
#include <string>

// Adds <exe dir>/lib to the DLL search path. Must run before any Qt or sherpa
// call.
static bool set_dll_directory() {
  wchar_t path[MAX_PATH];
  if (GetModuleFileNameW(nullptr, path, MAX_PATH) == 0) return false;
  std::wstring dir(path);
  dir.resize(dir.find_last_of(L'\\'));
  return SetDllDirectoryW((dir + L"\\lib").c_str()) != FALSE;
}

// Logs go to the parent terminal when started from one; double-click has none.
static void attach_parent_console() {
  if (!AttachConsole(ATTACH_PARENT_PROCESS)) return;
  FILE* f = nullptr;
  freopen_s(&f, "CONOUT$", "w", stdout);
  freopen_s(&f, "CONOUT$", "w", stderr);
  HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
  DWORD mode = 0;
  if (GetConsoleMode(out, &mode)) {
    SetConsoleMode(out, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
  }
}
#endif

// Loads the embedded fonts (src/assets).
static void load_fonts() {
  QFontDatabase::addApplicationFont(":/fonts/RobotoSlab-Regular.ttf");
  QFontDatabase::addApplicationFont(":/fonts/RobotoSlab-Bold.ttf");
  QFontDatabase::addApplicationFont(":/fonts/Molle-Regular.ttf");
}

int main(int argc, char* argv[]) {
#ifdef _WIN32
  if (!set_dll_directory()) return 1;
  attach_parent_console();
#endif
#ifdef __linux__
  qputenv("QT_QPA_PLATFORM", "xcb");
#endif

  log_setup();

  QApplication app(argc, argv);
  app.setApplicationName("Gladys");
  app.setWindowIcon(QIcon(":/icons/icon.svg"));
  app.setQuitOnLastWindowClosed(false);
  load_fonts();
  QFont ui_font("Roboto Slab");
  ui_font.setPixelSize(13);
  app.setFont(ui_font);
  qCInfo(lcMain) << "started";

  Setting* setting = Setting::instance();

  // QML engine lives for the whole run: splash, overlay and settings share it.
  AudioBars audio_bars;
  SettingsBridge settings_bridge;
  QQmlApplicationEngine engine;
  engine.rootContext()->setContextProperty("audioBars", &audio_bars);
  engine.rootContext()->setContextProperty("settingsBridge", &settings_bridge);

  engine.loadFromModule("Gladys", "Splash");
  if (engine.rootObjects().isEmpty()) {
    qCCritical(lcMain) << "failed to load splash";
    return 1;
  }
  QObject* splash = engine.rootObjects().first();

  // Model load is blocking: run it off the GUI thread so the splash keeps
  // spinning.
  STT* stt = new STT();
  bool stt_ready = false;
  QThread* loader = QThread::create([&]() {
    stt_ready = stt->init((QCoreApplication::applicationDirPath() +
      "/sherpaonnx/sherpa-onnx-nemo-parakeet-tdt-0.6b-v3-int8")
        .toStdString());
  });
  QEventLoop loading;
  QObject::connect(loader, &QThread::finished, &loading, &QEventLoop::quit);
  loader->start();
  loading.exec();
  loader->wait();
  delete loader;
  if (!stt_ready) {
    return 1;
  }

  LLM* llm = new LLM(&app);
  if (!llm->start()) {
    return 1;
  }

  Toggle* toggle = new Toggle();
  if (!toggle->init()) {
    return 1;
  }

  Paste* paste = new Paste();
  paste->init();

  QThread* stt_thread = new QThread(&app);
  QThread* toggle_thread = new QThread(&app);
  QThread* paste_thread = new QThread(&app);
  stt->moveToThread(stt_thread);
  toggle->moveToThread(toggle_thread);
  paste->moveToThread(paste_thread);
  QObject::connect(toggle_thread, &QThread::started, toggle, &Toggle::start_polling);
  stt_thread->start();
  toggle_thread->start();
  paste_thread->start();

  QEventLoop fading;
  QObject::connect(splash, SIGNAL(fadeOutFinished()), &fading, SLOT(quit()));
  QMetaObject::invokeMethod(splash, "fadeOut");
  fading.exec();
  splash->deleteLater();

  engine.loadFromModule("Gladys", "Overlay");
  if (engine.rootObjects().isEmpty()) {
    qCCritical(lcMain) << "failed to load overlay";
    return 1;
  }
  QObject* overlay = engine.rootObjects().last();
  auto* overlay_window = qobject_cast<QWindow*>(overlay);
  if (!overlay_window) {
    qCCritical(lcMain) << "overlay root is not a window";
    return 1;
  }
  remove_x11_shadow(overlay_window->winId());
#ifndef _WIN32  // Windows: the global hook needs no focus; activating steals it
  overlay_window->requestActivate();  // workaround: focus once so the hotkey is handled
#endif

  // Created after the overlay so the tray never appears during splash.
  Tray* tray = new Tray(&app);
  QObject::connect(tray, &Tray::SETTINGS_REQUESTED, overlay, [overlay]() {
    QMetaObject::invokeMethod(overlay, "openSettings");
  });

  bool recording = false;
  QObject::connect(toggle, &Toggle::KEY_PRESSED, &app, [&]() {
    recording = !recording;
    if (recording) {
      qCInfo(lcToggle) << "window open";
      QMetaObject::invokeMethod(stt, &STT::start_recording);
    } else {
      qCInfo(lcToggle) << "window close";
      QMetaObject::invokeMethod(stt, &STT::stop_recording);
    }
    overlay->setProperty("expanded", recording);
#ifndef _WIN32
    overlay_window->requestActivate();
#endif
  });

  // ASR -> LLM -> Text Copy -> Text Paste
  QObject::connect(stt, &STT::AUDIO_LEVEL_UPDATED, &audio_bars, &AudioBars::update_levels);
  QObject::connect(stt, &STT::FINAL_TEXT_READY, llm, [llm](const QString& text) {
    QTimer::singleShot(
      STT_TO_LLM_DELAY_MS, llm, [llm, text]() { llm->format_transcription(text); });
  });
  QObject::connect(llm, &LLM::FORMATTED_TEXT_READY, &app, [paste, &app](const QString& text) {
    // Context is the GUI thread: the clipboard must be touched there.
    QTimer::singleShot(LLM_TO_COPY_DELAY_MS, &app, [paste, text]() {
      Paste::copy_to_clipboard(text);
      QTimer::singleShot(COPY_TO_PASTE_DELAY_MS, paste, [paste]() {
        QMetaObject::invokeMethod(paste, &Paste::send_paste);
      });
    });
  });
  QObject::connect(setting, &Setting::SETTINGS_CHANGED, stt, &STT::reconfigure_audio_source);

  QObject::connect(&app, &QApplication::aboutToQuit, [&]() {
    qCInfo(lcMain) << "quitting";
    llm->stop();
    for (QThread* t : {stt_thread, toggle_thread, paste_thread}) {
      t->quit();
      t->wait();
    }
  });

#ifndef _WIN32
  if (pipe(g_signal_pipe) != 0) {
    qCCritical(lcMain) << "failed to create signal pipe";
    return 1;
  }
  std::signal(SIGINT, on_signal);
  std::signal(SIGTERM, on_signal);
  auto* signalNotifier = new QSocketNotifier(g_signal_pipe[0], QSocketNotifier::Read, &app);
  QObject::connect(signalNotifier, &QSocketNotifier::activated, &app, []() {
    char c;
    (void)!read(g_signal_pipe[0], &c, 1);
    QApplication::quit();
  });
#endif

  return app.exec();
}
