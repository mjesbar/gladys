// KeyGrab Module - Global hotkey detection.
// Cross-platform: X11 (Linux), CGEventTap (macOS), SetWindowsHookEx (Windows).

#ifndef KEYGRAB_HPP
#define KEYGRAB_HPP

#include <QObject>
#include <QDebug>
#include <QTimer>
#include <QElapsedTimer>

// Platform-specific includes are in keygrab.cc to avoid namespace pollution.

#ifdef __APPLE__
#include <CoreGraphics/CoreGraphics.h>
#endif
#ifdef _WIN32
#include <windows.h>
#endif

class KeyGrab : public QObject {
  Q_OBJECT

public:
  explicit KeyGrab(QObject *parent = nullptr);
  ~KeyGrab() override;

  bool init(void *display);
  unsigned int keycode() const;
  unsigned int modifiers() const;
  void *display() const;

public slots:
  void process_events();

signals:
  void key_pressed();

private:
  void *m_display;
  unsigned long m_root;
  unsigned int m_keycode;
  unsigned int m_modifiers;
  QElapsedTimer m_timer;

#ifdef __APPLE__
  void *m_event_tap;       // CFMachPortRef (opaque)
  void *m_run_loop_source;  // CFRunLoopSourceRef (opaque)
  static CGEventRef event_tap_callback(CGEventTapProxy proxy,
                                     CGEventType type,
                                     CGEventRef event,
                                     void *userInfo);
#endif

#ifdef _WIN32
  void *m_keyboard_hook;   // HHOOK (opaque)
  static LRESULT CALLBACK low_level_keyboard_proc(int nCode,
                                               WPARAM wParam,
                                               LPARAM lParam);
#endif
};

#endif // KEYGRAB_HPP
