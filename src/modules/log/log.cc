// Log - Qt logging categories, one per module, with a timestamped message pattern.

#include "log.hpp"

#include <QMetaObject>
#include <QString>
#include <QtGlobal>
#include <cstdio>
#include <cstring>

#include "../data/data.hpp"

// Every message is echoed to stderr and pushed to logs. Data's own messages are echoed only, so a
// failing store cannot loop.
static void handle_message(QtMsgType type, const QMessageLogContext& context, const QString& msg) {
  const QString line = qFormatLogMessage(type, context, msg);
  std::fprintf(stderr, "%s\n", qPrintable(line));
  if (std::strcmp(context.category, lcData().categoryName()) == 0) {
    return;
  }
  Data::post_log(QStringLiteral("[%1] %2").arg(context.category, msg),
    type == QtCriticalMsg || type == QtFatalMsg,
    type == QtWarningMsg);
}

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
  qInstallMessageHandler(handle_message);
}

Q_LOGGING_CATEGORY(lcMain, "gladys.main")
Q_LOGGING_CATEGORY(lcStt, "gladys.stt")
Q_LOGGING_CATEGORY(lcLlm, "gladys.llm")
Q_LOGGING_CATEGORY(lcToggle, "gladys.toggle")
Q_LOGGING_CATEGORY(lcPaste, "gladys.paste")
Q_LOGGING_CATEGORY(lcSetting, "gladys.setting")
Q_LOGGING_CATEGORY(lcUi, "gladys.ui")
Q_LOGGING_CATEGORY(lcData, "gladys.data")
