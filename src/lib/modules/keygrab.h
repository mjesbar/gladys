// KeyGrab Module - Global hotkey detection using X11.

#ifndef KEYGRAB_HPP
#define KEYGRAB_HPP

#include <QObject>
#include <QDebug>
#include <QTimer>

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
};

#endif // KEYGRAB_HPP