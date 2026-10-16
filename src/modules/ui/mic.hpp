// Mic Widget - Closed bar <-> open microphone with audio wave bars.

#ifndef UI_MIC_HPP
#define UI_MIC_HPP

#include <QPixmap>
#include <QVariantAnimation>
#include <QVector>
#include <QWidget>

class Mic : public QWidget {
  Q_OBJECT

public:
  static inline const QSize SIZE{119, 110}; // width: 20 bars * 5px + 19 gaps * 1px

  explicit Mic(QWidget *parent = nullptr);

  void set_expanded(bool expanded); // morphs bar <-> mic over 300ms
  bool is_animating() const;

public slots:
  void update_audio_levels(const QVector<double> &levels);

protected:
  void paintEvent(QPaintEvent *event) override;

private:
  void draw_circle(QPainter &p);
  void draw_audio_wave(QPainter &p, double scale, double centerY);
  void draw_shadow(QPainter &p, int x, int y, int micSize);

  QVariantAnimation *m_morph_animation;
  double m_progress; // 0.0 = idle circle, 1.0 = recording (mic open)
  bool m_is_expanded;
  QVector<double> m_audio_levels;
  QPixmap m_mic_pixmap;        // full-size icon, rendered once from SVG
  QPixmap m_scaled_mic_pixmap; // cache for the current scale
  QPixmap m_shadow_pixmap;     // blurred yellow halo, cache per size
  QPixmap m_outline_pixmap;    // grey 2px border of the icon, cache per size
  int m_shadow_size;
};

#endif // UI_MIC_HPP
