// Gladys - Entry point. Starts the module threads, the core and the UI. Owns shutdown.

#include <QApplication>
#include <QFont>
#include <QFontDatabase>
#include <QIcon>
#include <QEventLoop>
#include <vector>
#include <QMetaObject>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSocketNotifier>
#include <QThread>
#include <QWindow>

#include "core.hpp"
#include "modules/keypaste/paste.hpp"
#include "modules/keytoggle/toggle.hpp"
#include "modules/audio/levels.hpp"
#include "modules/log/log.hpp"
#include "modules/settings/setting.hpp"
#include "modules/ui/waves.hpp"
#include "modules/ui/settings.hpp"
#include "modules/ui/tray.hpp"
#include "modules/ui/osshadow.hpp"
#ifdef __APPLE__
#include "modules/ui/macos.hpp"
#endif

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

// Adds <exe dir>/lib to the DLL search path. Must run before any Qt call.
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

  // Each module runs on its own thread. Settings owns QSettings and the device list.
  Setting* setting = new Setting();
  QThread* settings_thread = new QThread(&app);
  setting->moveToThread(settings_thread);

  // The hotkey hook is installed on its own thread by Toggle::start.
  Toggle* toggle = new Toggle();
  QThread* toggle_thread = new QThread(&app);
  toggle->moveToThread(toggle_thread);
  QObject::connect(toggle_thread, &QThread::started, toggle, &Toggle::start);
  QObject::connect(toggle, &Toggle::STARTED, &app, [&app](bool ok) {
    if (!ok) {
      qCCritical(lcMain) << "hotkey unavailable";
      app.exit(1);
    }
  });

  // Keystrokes run on their own thread; the clipboard stays on the GUI thread.
  Paste* paste = new Paste();
  paste->init();
  QThread* paste_thread = new QThread(&app);
  paste->moveToThread(paste_thread);

  for (QThread* t : {settings_thread, toggle_thread, paste_thread}) {
    t->start();
  }
  // Stops the module threads on every return path, including startup failures.
  struct ThreadStopper {
    std::vector<QThread*> threads;
    ~ThreadStopper() {
      qCInfo(lcMain) << "quitting";
      for (QThread* t : threads) {
        t->quit();
        t->wait();
      }
    }
  } stop_threads{{settings_thread, toggle_thread, paste_thread}};

  // Settings bridge calls the settings thread, so it starts after the thread.
  Waves waves;
  AudioLevels levels;  // wave bars read the mic directly, not the ASR process
  if (!levels.init()) {
    return 1;
  }
  SettingsBridge settings_bridge(setting);
  QQmlApplicationEngine engine;
  engine.rootContext()->setContextProperty("waves", &waves);
  engine.rootContext()->setContextProperty("settingsBridge", &settings_bridge);

  engine.loadFromModule("Gladys", "Splash");
  if (engine.rootObjects().isEmpty()) {
    qCCritical(lcMain) << "failed to load splash";
    return 1;
  }
  QObject* splash = engine.rootObjects().first();

  // gladys-asr and gladys-llm are separate processes owned by the core.
  const QString model_dir = QCoreApplication::applicationDirPath() +
    "/sherpaonnx/sherpa-onnx-nemo-parakeet-tdt-0.6b-v3-int8";
  Core core(model_dir, paste);
  QObject::connect(setting, &Setting::SETTINGS_CHANGED, &core, &Core::update_config);
  QObject::connect(toggle, &Toggle::KEY_PRESSED, &core, &Core::toggle_recording);
  QObject::connect(setting, &Setting::SETTINGS_CHANGED, &levels, [&levels](const QVariantMap& s) {
    levels.set_source(s.value("inputSource").toString());
  });
  QObject::connect(&levels, &AudioLevels::LEVELS_UPDATED, &waves, &Waves::update_levels);
  QObject::connect(&core, &Core::RECORDING_CHANGED, &levels, &AudioLevels::set_active);
  // Publish after the connections exist, so the initial settings are not lost.
  QMetaObject::invokeMethod(setting, "publish");
  QObject::connect(&core, &Core::FAILED, &app, [&app](const QString& reason) {
    qCCritical(lcMain) << reason;
    app.exit(1);
  });
  if (!core.start()) {
    return 1;
  }

  // Splash stays until both APIs are ready; a failed API aborts the start.
  QEventLoop starting;
  QObject::connect(&core, &Core::READY, &starting, &QEventLoop::quit);
  QObject::connect(&core, &Core::FAILED, &starting, [&starting]() { starting.exit(1); });
  if (starting.exec() != 0) {
    return 1;
  }

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
#ifdef __APPLE__
  configure_macos_overlay(overlay_window->winId());
#endif
#ifndef _WIN32  // Windows: the global hook needs no focus; activating steals it
  overlay_window->requestActivate();  // workaround: focus once so the hotkey is handled
#endif

  // Created after the overlay so the tray never appears during splash.
  Tray* tray = new Tray(&app);
  QObject::connect(tray, &Tray::SETTINGS_REQUESTED, overlay, [overlay]() {
    QMetaObject::invokeMethod(overlay, "openSettings");
  });

  QObject::connect(&core, &Core::RECORDING_CHANGED, &app, [overlay, overlay_window](bool recording) {
    qCInfo(lcToggle) << (recording ? "window open" : "window close");
    overlay->setProperty("recording", recording);
#ifndef _WIN32
    overlay_window->requestActivate();
#endif
  });
  QObject::connect(&core, &Core::PROCESSING_CHANGED, overlay, [overlay](bool processing) {
    overlay->setProperty("processing", processing);
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
