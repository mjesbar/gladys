// History Menu - Store Transcriptions and Store Logs toggles. Views read Data (see ui/history.hpp).

#ifndef SETTING_HISTORY_HPP
#define SETTING_HISTORY_HPP

#include <QVariantMap>

class History {
 public:
  QVariantMap to_map() const;
  void from_map(const QVariantMap& values);

 private:
  bool m_store_transcriptions = true;
  bool m_store_logs = true;
};

#endif  // SETTING_HISTORY_HPP
