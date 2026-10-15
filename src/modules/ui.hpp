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

class SettingsWindow;

class UI : public QWidget {
  Q_OBJECT

public:
  explicit UI(QWidget *parent = nullptr);
  ~UI() override;

  void reposition_on_primary_screen();

public slots:
  void toggle_visibility();
  void handle_opacity_animation_finished();
  void remove_shadow();
  void quit_app();
  void update_audio_levels(const QVector<double> &levels);
  void open_settings();

signals:
  void quit_requested();

protected:
  void paintEvent(QPaintEvent *event) override;

private:
  QPropertyAnimation *m_position_animation;
  QPropertyAnimation *m_opacity_animation;
  QPropertyAnimation *m_size_animation;
  int m_target_y_visible;
  int m_target_y_subtle;
  bool m_is_prominent;
  QSize m_original_size;
  QPoint m_prominent_pos;
  QPoint m_subtle_pos;
  QSystemTrayIcon *m_tray_icon;
  QMenu *m_tray_menu;
  QVector<double> m_audio_levels;
  QVariantAnimation *m_scale_animation;
  QPixmap m_mic_pixmap;
  QPixmap m_scaled_mic_pixmap;
  SettingsWindow *m_settings_window;

  void draw_audio_wave(QPainter &p, const QSize &size, double scale);
};

#endif // UI_HPP
