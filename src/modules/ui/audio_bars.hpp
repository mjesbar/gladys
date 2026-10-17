// Audio Bars - Turns STT audio levels into the 20 wave bar heights for QML.

#ifndef UI_AUDIO_BARS_HPP
#define UI_AUDIO_BARS_HPP

#include <QList>
#include <QObject>
#include <QVector>

class AudioBars : public QObject {
  Q_OBJECT
  Q_PROPERTY(QList<qreal> heights READ heights NOTIFY HEIGHTS_CHANGED)

public:
  static constexpr int BAR_COUNT = 20;

  explicit AudioBars(QObject *parent = nullptr);

  QList<qreal> heights() const;

public slots:
  void update_levels(const QVector<double> &levels);

signals:
  void HEIGHTS_CHANGED();

private:
  QList<qreal> m_heights; // pixel heights at full (open) scale
};

#endif // UI_AUDIO_BARS_HPP
