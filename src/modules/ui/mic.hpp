// Mic Widget - Microphone icon with audio wave bars.

#ifndef UI_MIC_HPP
#define UI_MIC_HPP

#include <QPixmap>
#include <QVariantAnimation>
#include <QVector>
#include <QWidget>

class Mic : public QWidget {
  Q_OBJECT

public:
  static inline const QSize SIZE{119, 60}; // 20 bars * 5px + 19 gaps * 1px

  explicit Mic(QWidget *parent = nullptr);

  void set_expanded(bool expanded); // animates icon scale 0.5 <-> 1.0

public slots:
  void update_audio_levels(const QVector<double> &levels);

protected:
  void paintEvent(QPaintEvent *event) override;

private:
  void draw_audio_wave(QPainter &p, double scale);
  void draw_shadow(QPainter &p, int x, int y, int micSize);

  QVariantAnimation *m_scale_animation;
  bool m_is_expanded;
  QVector<double> m_audio_levels;
  QPixmap m_mic_pixmap;        // full-size icon, rendered once from SVG
  QPixmap m_scaled_mic_pixmap; // cache for the current scale
  QPixmap m_shadow_pixmap;     // blurred yellow halo, cache per size
  int m_shadow_size;
};

#endif // UI_MIC_HPP
