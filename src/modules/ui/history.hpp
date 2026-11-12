// History Bridge - Exposes Data reads (Activity, Status, Errors, Logs) to the Settings QML window.

#ifndef UI_HISTORY_BRIDGE_HPP
#define UI_HISTORY_BRIDGE_HPP

#include <QObject>
#include <QVariantList>

class HistoryBridge : public QObject {
  Q_OBJECT

 public:
  // Data lives on its own thread; every call blocks until it answers.
  explicit HistoryBridge(QObject* parent = nullptr);

  Q_INVOKABLE QVariantList transcriptions(int offset, int limit) const;
  Q_INVOKABLE QVariantList logs(int offset, int limit) const;
  Q_INVOKABLE int error_count() const;  // errors in the last 30 days
  Q_INVOKABLE int health() const;       // 100 minus the error share of the last 30 days, in percent

 signals:
  void transcriptionsChanged();  // a transcription was stored; emitted on the GUI thread
  void logsChanged();            // a log line was stored; emitted on the GUI thread
};

#endif  // UI_HISTORY_BRIDGE_HPP
