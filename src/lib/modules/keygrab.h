// KeyGrab Module - Global hotkey detection.
// Cross-platform: X11 (Linux), CGEventTap (macOS), SetWindowsHookEx (Windows).

#ifndef KEYGRAB_HPP
#define KEYGRAB_HPP

#include <QObject>
#include <QDebug>
#include <QTimer>
#include <QElapsedTimer>

// Platform-specific includes are in keygrab.cc to avoid namespace pollution.

class KeyGrab : public QObject {
  Q_OBJECT

public:
  explicit KeyGrab(QObject *parent = nullptr);
  ~KeyGrab() override;

  bool init(void *display);
  unsigned int keycode() const { return m_keycode; }
  unsigned int modifiers() const { return m_modifiers; }
  void *display() const { return m_display; }

public slots:
  void processEvents();

signals:
  void keyPressed();

private:
  void *m_display;
  unsigned long m_root;
  unsigned int m_keycode;
  unsigned int m_modifiers;
  QElapsedTimer m_timer;

#ifdef __APPLE__
  void *m_eventTap;       // CFMachPortRef (opaque)
  void *m_runLoopSource;  // CFRunLoopSourceRef (opaque)
  static void *eventTapCallback(void *proxy, int type,
                                 void *event, void *userInfo);
#endif

#ifdef _WIN32
  void *m_keyboardHook;   // HHOOK (opaque)
  static long __stdcall lowLevelKeyboardProc(int nCode,
                                              unsigned long wParam,
                                              long lParam);
#endif
};

#endif // KEYGRAB_HPP
