#ifndef WINDOW_HPP
#define WINDOW_HPP

#include <QApplication>
#include <QEasingCurve>
#include <QMenu>
#include <QPropertyAnimation>
#include <QSize>
#include <QSystemTrayIcon>
#include <QWidget>

class GladysWindow : public QWidget {
  Q_OBJECT

public:
  explicit GladysWindow(QWidget *parent = nullptr);
  ~GladysWindow() override;

public slots:
  void toggleVisibility();
  void handleOpacityAnimationFinished();
  void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
  void removeShadow();

protected:
  void paintEvent(QPaintEvent *event) override;

private:
  QPropertyAnimation *m_positionAnimation;
  QPropertyAnimation *m_opacityAnimation;
  QPropertyAnimation *m_sizeAnimation;
  int m_targetYVisible;
  int m_targetYSubtle;
  bool m_isProminent;
  QSize m_originalSize;
  QSystemTrayIcon *m_trayIcon;
  QMenu *m_trayMenu;
};

#endif // GLADYSWINDOW_HPP
