// Log - Qt logging categories, one per module, with a timestamped message pattern.

#include "log.hpp"

#include <QString>
#include <QtGlobal>

// ANSI colors per level: DEBUG grey, INFO green, WARN yellow, ERROR red, FATAL magenta
void log_setup() {
  QLoggingCategory::setFilterRules(QStringLiteral("gladys.*=true"));  // info is off by default
  qSetMessagePattern(
    "%{time yyyy-MM-dd hh:mm:ss.zzz} "
    "%{if-debug}\033[90mDEBUG%{endif}"
    "%{if-info}\033[32mINFO%{endif}"
    "%{if-warning}\033[33mWARN%{endif}"
    "%{if-critical}\033[31mERROR%{endif}"
    "%{if-fatal}\033[35mFATAL%{endif}"
    "\033[0m [%{category}] %{message}");
}

Q_LOGGING_CATEGORY(lcMain, "gladys.main")
Q_LOGGING_CATEGORY(lcStt, "gladys.stt")
Q_LOGGING_CATEGORY(lcLlm, "gladys.llm")
Q_LOGGING_CATEGORY(lcToggle, "gladys.toggle")
Q_LOGGING_CATEGORY(lcPaste, "gladys.paste")
Q_LOGGING_CATEGORY(lcSetting, "gladys.setting")
Q_LOGGING_CATEGORY(lcUi, "gladys.ui")
