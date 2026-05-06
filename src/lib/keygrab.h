#ifndef KEYGRAB_HPP
#define KEYGRAB_HPP

#include <QObject>
#include <QDebug>
#include <X11/Xlib.h>

class X11KeyGrab : public QObject {
  Q_OBJECT

public:
  explicit X11KeyGrab(QObject *parent = nullptr);
  ~X11KeyGrab() override;

  bool init(Display *display);
  KeyCode keycode() const { return m_keycode; }
  unsigned int modifiers() const { return m_modifiers; }
  Display *display() const { return m_display; }

signals:
  void keyPressed();

public slots:
  void processEvents();

private:
  Display *m_display;
  Window m_root;
  KeyCode m_keycode;
  unsigned int m_modifiers;
};

#endif // KEYGRAB_HPP