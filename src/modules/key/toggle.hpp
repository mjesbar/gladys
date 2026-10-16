// Toggle Module - Global hotkey (Ctrl+Alt+P) detection.
// Cross-platform: X11 (Linux), CGEventTap (macOS), SetWindowsHookEx (Windows).

#ifndef KEY_TOGGLE_HPP
#define KEY_TOGGLE_HPP

#include <QObject>
#include <QTimer>

#ifdef __APPLE__
#include <CoreGraphics/CoreGraphics.h>
#endif
#ifdef _WIN32
#include <windows.h>
#endif

class Toggle : public QObject {
  Q_OBJECT

public:
  explicit Toggle(QObject *parent = nullptr);
  ~Toggle() override;

  bool init();

public slots:
  void start_polling();
  void process_events();

signals:
  void KEY_PRESSED();

private:
  QTimer *m_poll_timer;
  void *m_display; // Display* (X11)
  unsigned int m_keycode;
  unsigned int m_modifiers;

#ifdef __APPLE__
  void *m_event_tap;      // CFMachPortRef (opaque)
  void *m_run_loop_source; // CFRunLoopSourceRef (opaque)
  static CGEventRef event_tap_callback(CGEventTapProxy proxy, CGEventType type,
                                       CGEventRef event, void *userInfo);
#endif

#ifdef _WIN32
  void *m_keyboard_hook; // HHOOK (opaque)
  static LRESULT CALLBACK low_level_keyboard_proc(int nCode, WPARAM wParam,
                                                  LPARAM lParam);
#endif
};

#endif // KEY_TOGGLE_HPP
