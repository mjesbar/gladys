// Paste Module - Sends Ctrl+Shift+V (plain-text paste) to the focused window.
// Cross-platform: uinput (Linux), CGEvent (macOS), SendInput (Windows).

#ifndef KEY_PASTE_HPP
#define KEY_PASTE_HPP

#include <QObject>
#include <QString>

class Paste : public QObject {
  Q_OBJECT

 public:
  explicit Paste(QObject* parent = nullptr);
  ~Paste() override;

  void init();
  static void copy_to_clipboard(const QString& text);  // call on GUI thread

 public slots:
  void send_paste();

 private:
  void send_key(unsigned int code, bool press);

#ifdef __linux__
  int m_fd;
#endif
};

#endif  // KEY_PASTE_HPP
