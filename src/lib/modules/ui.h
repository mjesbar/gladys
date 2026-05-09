#ifndef UI_HPP
#define UI_HPP

#include <QApplication>
#include <QEasingCurve>
#include <QMenu>
#include <QPropertyAnimation>
#include <QSize>
#include <QSystemTrayIcon>
#include <QVector>
#include <QWidget>
#include <QPixmap>

class UI : public QWidget {
  Q_OBJECT

public:
  explicit UI(QWidget *parent = nullptr);
  ~UI() override;

public slots:
  void toggleVisibility();
  void handleOpacityAnimationFinished();
  void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
  void removeShadow();
  void quitApp();
  void updateAudioLevels(const QVector<double> &levels);

signals:
  void quitRequested();

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
  QVector<double> m_audioLevels;
  QVariantAnimation *m_scaleAnimation;
  QPixmap m_micPixmap;
  QPixmap m_scaledMicPixmap;

private:
  void drawAudioWave(QPainter &p, const QSize &size, double scale);
};

#endif // UI_HPP
