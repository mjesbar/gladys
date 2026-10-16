// Splash Window - Centered logo with a blue loading spinner, fades in and out.

#ifndef UI_SPLASH_WINDOW_HPP
#define UI_SPLASH_WINDOW_HPP

#include <QPixmap>
#include <QPropertyAnimation>
#include <QTimer>
#include <QWidget>

class SplashWindow : public QWidget {
  Q_OBJECT

public:
  explicit SplashWindow(QWidget *parent = nullptr);

  void fade_in();
  void fade_out(); // hides the window, then emits FADE_OUT_FINISHED

signals:
  void FADE_OUT_FINISHED();

protected:
  void paintEvent(QPaintEvent *event) override;

private:
  QPropertyAnimation *m_opacity_animation;
  QTimer *m_spin_timer;
  QPixmap m_logo;
  int m_spin_angle;
};

#endif // UI_SPLASH_WINDOW_HPP
