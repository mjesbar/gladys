// Toggle Module - Global hotkey for the active profile (Ctrl+Alt+P, Fn or AltGr).
// Cross-platform: evdev /dev/input (Linux), CGEventTap (macOS), SetWindowsHookEx (Windows).

#ifndef KEY_TOGGLE_HPP
#define KEY_TOGGLE_HPP

#include <QObject>
#include <QElapsedTimer>
#include <QString>
#include <QTimer>

#include <atomic>

#ifdef __linux__
#include <vector>
#endif

#ifdef __APPLE__
#include <CoreGraphics/CoreGraphics.h>
#endif
#ifdef _WIN32
#include <windows.h>
#endif

enum class HotkeyProfile { CtrlAltP, Fn, AltGr };

class Toggle : public QObject {
  Q_OBJECT

 public:
  explicit Toggle(QObject* parent = nullptr);
  ~Toggle() override;

  // Any thread: the profile is atomic, so the keytoggle thread reads it on the next key event.
  void set_profile(const QString& name);

 public slots:
  void start();  // call on the keytoggle thread: the hook must live on it
  void process_events();
#ifdef __APPLE__
  void stop();  // any thread: ends the macOS loop run by start()
#endif

 signals:
  void STARTED(bool ok);
  void KEY_PRESSED();   // once per press; key repeat is ignored
  void KEY_RELEASED();  // once per release of the trigger

 private:
  bool init();
  void on_trigger(bool down);

  QTimer* m_poll_timer;
  std::atomic<HotkeyProfile> m_profile{HotkeyProfile::CtrlAltP};
  std::atomic<bool> m_held{false};  // trigger is down
  QElapsedTimer m_edge_clock;       // time since the last accepted press or release

#ifdef __linux__
  std::vector<int> m_fds;  // keyboard devices in /dev/input
  bool m_ctrl_down = false;
  bool m_alt_down = false;
#endif

#ifdef __APPLE__
  void* m_event_tap;        // CFMachPortRef (opaque)
  void* m_run_loop_source;  // CFRunLoopSourceRef (opaque)
  std::atomic<bool> m_stop_requested;
  static CGEventRef event_tap_callback(
    CGEventTapProxy proxy, CGEventType type, CGEventRef event, void* userInfo);
#endif

#ifdef _WIN32
  void* m_keyboard_hook;  // HHOOK (opaque)
  static LRESULT CALLBACK low_level_keyboard_proc(int nCode, WPARAM wParam, LPARAM lParam);
#endif
};

#endif  // KEY_TOGGLE_HPP
