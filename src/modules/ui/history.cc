// History Bridge - Exposes Data reads (Activity, Status, Errors, Logs) to the Settings QML window.

#include "history.hpp"

#include <QMetaObject>
#include <QtGlobal>

#include "../data/data.hpp"

static int call_count(const char* slot) {
  int count = 0;
  QMetaObject::invokeMethod(
    Data::instance(), slot, Qt::BlockingQueuedConnection, Q_RETURN_ARG(int, count));
  return count;
}

HistoryBridge::HistoryBridge(QObject* parent) : QObject(parent) {}

QVariantList HistoryBridge::transcriptions(int offset, int limit) const {
  QVariantList rows;
  QMetaObject::invokeMethod(Data::instance(),
    "read_history",
    Qt::BlockingQueuedConnection,
    Q_RETURN_ARG(QVariantList, rows),
    Q_ARG(int, offset),
    Q_ARG(int, limit));
  return rows;
}

QVariantList HistoryBridge::logs(int offset, int limit) const {
  QVariantList rows;
  QMetaObject::invokeMethod(Data::instance(),
    "read_logs",
    Qt::BlockingQueuedConnection,
    Q_RETURN_ARG(QVariantList, rows),
    Q_ARG(int, offset),
    Q_ARG(int, limit));
  return rows;
}

int HistoryBridge::error_count() const { return call_count("count_error_logs"); }

int HistoryBridge::health() const {
  const int total = call_count("count_logs");
  if (total == 0) {
    return 100;
  }
  return qRound(100.0 * (total - error_count()) / total);
}
