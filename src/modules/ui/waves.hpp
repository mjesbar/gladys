// Waves - Turns audio levels into the 20 wave bar heights for QML.

#ifndef UI_WAVES_HPP
#define UI_WAVES_HPP

#include <QList>
#include <QObject>
#include <QVector>

class Waves : public QObject {
  Q_OBJECT
  Q_PROPERTY(QList<qreal> heights READ heights NOTIFY HEIGHTS_CHANGED)

 public:
  static constexpr int BAR_COUNT = 20;

  explicit Waves(QObject* parent = nullptr);

  QList<qreal> heights() const;

 public slots:
  void update_levels(const QVector<double>& levels);

 signals:
  void HEIGHTS_CHANGED();

 private:
  QList<qreal> m_heights;  // pixel heights at full (open) scale
};

#endif  // UI_WAVES_HPP
