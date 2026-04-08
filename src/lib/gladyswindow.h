#ifndef GLADYSWINDOW_HPP
#define GLADYSWINDOW_HPP

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

protected:
  void paintEvent(QPaintEvent *event) override;

private:
  QPropertyAnimation *m_positionAnimation;
  QPropertyAnimation *m_opacityAnimation;
  int m_targetYVisible;
  int m_targetYSubtle;
  bool m_isProminent;
  QSize m_originalSize;
  QSystemTrayIcon *m_trayIcon;
  QMenu *m_trayMenu;
};

#endif // GLADYSWINDOW_HPP