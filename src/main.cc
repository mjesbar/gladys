// Gladys - Entry point. Creates the modules, wires their signals and owns shutdown.

#include <QApplication>
#include <QMetaObject>
#include <QSocketNotifier>
#include <QThread>


#include "modules/log/log.hpp"
#include "modules/key/paste.hpp"
#include "modules/key/toggle.hpp"
#include "modules/ai/llm.hpp"
#include "modules/setting/setting.hpp"
#include "modules/ai/stt.hpp"
#include "modules/ui/app_window.hpp"

#ifndef _WIN32
#include <csignal>
#include <unistd.h>

static int g_signal_pipe[2];

// Async-signal-safe: only writes a byte; the Qt loop does the real shutdown.
static void on_signal(int) {
  char c = 0;
  (void)!write(g_signal_pipe[1], &c, 1);
}
#endif

int main(int argc, char *argv[]) {
#ifdef __linux__
  qputenv("QT_QPA_PLATFORM", "xcb");
#endif

  log_setup();

  QApplication app(argc, argv);
  app.setApplicationName("Gladys");
  app.setQuitOnLastWindowClosed(false);
  qCInfo(lcMain) << "started";

  Setting *setting = Setting::instance();

  STT *stt = new STT();
  if (!stt->init((QCoreApplication::applicationDirPath() +
                  "/sherpaonnx/sherpa-onnx-streaming-zipformer-es-kroko-2025-08-06")
                     .toStdString())) {
    return 1;
  }

  LLM *llm = new LLM(&app);
  if (!llm->start()) {
    return 1;
  }

  Toggle *toggle = new Toggle();
  if (!toggle->init()) {
    return 1;
  }

  Paste *paste = new Paste();
  paste->init();

  QThread *stt_thread = new QThread(&app);
  QThread *toggle_thread = new QThread(&app);
  QThread *paste_thread = new QThread(&app);
  stt->moveToThread(stt_thread);
  toggle->moveToThread(toggle_thread);
  paste->moveToThread(paste_thread);
  QObject::connect(toggle_thread, &QThread::started, toggle, &Toggle::start_polling);
  stt_thread->start();
  toggle_thread->start();
  paste_thread->start();

  AppWindow window;
  window.show();
  window.remove_shadow();

  bool recording = false;
  QObject::connect(toggle, &Toggle::KEY_PRESSED, &window, [&]() {
    recording = !recording;
    if (recording) {
      qCInfo(lcToggle) << "window open";
      QMetaObject::invokeMethod(stt, &STT::start_recording);
    } else {
      qCInfo(lcToggle) << "window close";
      QMetaObject::invokeMethod(stt, &STT::stop_recording);
    }
    window.toggle_visibility();
  });

  QObject::connect(stt, &STT::AUDIO_LEVEL_UPDATED, window.mic(), &Mic::update_audio_levels);
  QObject::connect(stt, &STT::FINAL_TEXT_READY, llm, &LLM::format_transcription);
  QObject::connect(llm, &LLM::FORMATTED_TEXT_READY, &window, [paste](const QString &text) {
    Paste::copy_to_clipboard(text);
    QMetaObject::invokeMethod(paste, &Paste::send_paste);
  });
  QObject::connect(setting, &Setting::SETTINGS_CHANGED, stt, &STT::reconfigure_audio_source);

  QObject::connect(&app, &QApplication::aboutToQuit, [&]() {
    qCInfo(lcMain) << "quitting";
    llm->stop();
    for (QThread *t : {stt_thread, toggle_thread, paste_thread}) {
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
  auto *signalNotifier = new QSocketNotifier(g_signal_pipe[0], QSocketNotifier::Read, &app);
  QObject::connect(signalNotifier, &QSocketNotifier::activated, &app, []() {
    char c;
    (void)!read(g_signal_pipe[0], &c, 1);
    QApplication::quit();
  });
#endif

  return app.exec();
}
