// History Menu - Store Transcriptions and Store Logs toggles. Views read Data (see ui/history.hpp).

#include "history.hpp"

QVariantMap History::to_map() const {
  return {{"storeTranscriptions", m_store_transcriptions}, {"storeLogs", m_store_logs}};
}

void History::from_map(const QVariantMap& values) {
  m_store_transcriptions = values.value("storeTranscriptions", m_store_transcriptions).toBool();
  m_store_logs = values.value("storeLogs", m_store_logs).toBool();
}
