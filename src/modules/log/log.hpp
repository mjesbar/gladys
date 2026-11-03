// Log - Qt logging categories, one per module, with a timestamped message pattern.

#ifndef LIB_LOG_HPP
#define LIB_LOG_HPP

#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(lcMain)
Q_DECLARE_LOGGING_CATEGORY(lcStt)
Q_DECLARE_LOGGING_CATEGORY(lcLlm)
Q_DECLARE_LOGGING_CATEGORY(lcToggle)
Q_DECLARE_LOGGING_CATEGORY(lcPaste)
Q_DECLARE_LOGGING_CATEGORY(lcSetting)
Q_DECLARE_LOGGING_CATEGORY(lcUi)
Q_DECLARE_LOGGING_CATEGORY(lcData)

void log_setup();  // timestamped, colored output and storage in logs; call once at startup

#endif  // LIB_LOG_HPP
