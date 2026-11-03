// Data - Singleton owner of history.db (SQLite, plaintext). ORM-style CRUD per table, on its own thread.

#include "data.hpp"

#include <QDateTime>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QMetaObject>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QStandardPaths>
#include <QTimer>
#include <atomic>

#include "../log/log.hpp"

static const char* const CONNECTION = "gladys";
static const int RETENTION_DAYS = 30;
static const int PURGE_INTERVAL_MS = 60 * 60 * 1000;

static const char* const SCHEMA[] = {
    "CREATE TABLE IF NOT EXISTS history (id INTEGER PRIMARY KEY AUTOINCREMENT, datetime TEXT NOT NULL, "
    "transcription TEXT NOT NULL)",
    "CREATE INDEX IF NOT EXISTS history_datetime ON history (datetime)",
    "CREATE TABLE IF NOT EXISTS logs (id INTEGER PRIMARY KEY AUTOINCREMENT, datetime TEXT NOT NULL, "
    "log TEXT NOT NULL, is_err INTEGER NOT NULL, is_warn INTEGER NOT NULL)",
    "CREATE INDEX IF NOT EXISTS logs_datetime ON logs (datetime)",
    "CREATE TABLE IF NOT EXISTS settings (id INTEGER PRIMARY KEY AUTOINCREMENT, key TEXT UNIQUE NOT NULL, "
    "value TEXT NOT NULL)",
    "CREATE TABLE IF NOT EXISTS dictionary (id INTEGER PRIMARY KEY AUTOINCREMENT, from_text TEXT UNIQUE NOT NULL, "
    "to_text TEXT NOT NULL)",
    "CREATE TABLE IF NOT EXISTS snippets (id INTEGER PRIMARY KEY AUTOINCREMENT, from_text TEXT UNIQUE NOT NULL, "
    "to_text TEXT NOT NULL)",
};

static std::atomic<Data*> g_instance{nullptr};

static QString now_iso() { return QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs); }

static QString cutoff_iso() {
  return QDateTime::currentDateTimeUtc().addDays(-RETENTION_DAYS).toString(Qt::ISODateWithMs);
}

// Values keep their type (bool stays bool): stored as a JSON document, decoded back on read.
static QString encode(const QVariant& value) {
  return QString::fromUtf8(
      QJsonDocument(QJsonObject{{"v", QJsonValue::fromVariant(value)}}).toJson(QJsonDocument::Compact));
}

static QVariant decode(const QString& text) {
  return QJsonDocument::fromJson(text.toUtf8()).object().value("v").toVariant();
}

// Every row becomes a map keyed by column name.
static QVariantList to_rows(QSqlQuery& q) {
  QVariantList rows;
  while (q.next()) {
    const QSqlRecord record = q.record();
    QVariantMap row;
    for (int i = 0; i < record.count(); ++i) {
      row.insert(record.fieldName(i), q.value(i));
    }
    rows << row;
  }
  return rows;
}

Data* Data::create() {
  if (!g_instance) {
    g_instance = new Data();
  }
  return g_instance;
}

Data* Data::instance() { return g_instance; }

Data::~Data() {
  if (g_instance == this) {
    g_instance = nullptr;
  }
}

void Data::post_log(const QString& text, bool is_err, bool is_warn) {
  if (Data* data = g_instance) {
    QMetaObject::invokeMethod(data, "push_log", Qt::QueuedConnection, Q_ARG(QString, text), Q_ARG(bool, is_err),
                              Q_ARG(bool, is_warn));
  }
}

void Data::post_transcription(const QString& text) {
  if (Data* data = g_instance) {
    QMetaObject::invokeMethod(data, "push_transcription", Qt::QueuedConnection, Q_ARG(QString, text));
  }
}

// Opens history.db in the per-user app data folder, creates the schema and purges old logs.
bool Data::open() {
  const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  if (!QDir().mkpath(dir)) {
    fail("open", "cannot create " + dir);
    return false;
  }
  QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", CONNECTION);
  db.setDatabaseName(dir + "/history.db");
  if (!db.open()) {
    fail("open", db.lastError().text());
    return false;
  }
  m_open = true;
  query("schema", "PRAGMA journal_mode = WAL");
  for (const char* sql : SCHEMA) {
    query("schema", sql);
  }
  purge_logs();
  m_purge = new QTimer(this);
  connect(m_purge, &QTimer::timeout, this, &Data::purge_logs);
  m_purge->start(PURGE_INTERVAL_MS);
  qCInfo(lcData) << "opened" << db.databaseName();
  return true;
}

// Runs after queued writes, which are ahead of it in the thread's queue.
void Data::close() {
  m_open = false;
  if (m_purge) {
    m_purge->stop();
  }
  QSqlDatabase::database(CONNECTION).close();
  QSqlDatabase::removeDatabase(CONNECTION);
}

void Data::set_toggles(bool store_transcriptions, bool store_logs) {
  m_store_transcriptions = store_transcriptions;
  m_store_logs = store_logs;
}

void Data::push_transcription(const QString& text) {
  if (m_store_transcriptions) {
    create_history(text);
  }
}

void Data::push_log(const QString& text, bool is_err, bool is_warn) {
  if (m_store_logs) {
    create_logs(text, is_err, is_warn);
  }
}

// history

qlonglong Data::create_history(const QString& transcription) {
  QSqlQuery q = query("history", "INSERT INTO history (datetime, transcription) VALUES (?, ?)",
                      {now_iso(), transcription});
  return q.isActive() ? q.lastInsertId().toLongLong() : -1;
}

QVariantList Data::read_history(int offset, int limit) {
  QSqlQuery q = query("history",
                      "SELECT id, datetime, transcription FROM history ORDER BY datetime DESC LIMIT ? OFFSET ?",
                      {limit, offset});
  return to_rows(q);
}

bool Data::update_history(qlonglong id, const QString& transcription) {
  QSqlQuery q = query("history", "UPDATE history SET transcription = ? WHERE id = ?", {transcription, id});
  return q.isActive() && q.numRowsAffected() > 0;
}

bool Data::delete_history(qlonglong id) {
  QSqlQuery q = query("history", "DELETE FROM history WHERE id = ?", {id});
  return q.isActive() && q.numRowsAffected() > 0;
}

// logs

qlonglong Data::create_logs(const QString& text, bool is_err, bool is_warn) {
  QSqlQuery q = query("logs", "INSERT INTO logs (datetime, log, is_err, is_warn) VALUES (?, ?, ?, ?)",
                      {now_iso(), text, is_err, is_warn});
  return q.isActive() ? q.lastInsertId().toLongLong() : -1;
}

QVariantList Data::read_logs(int offset, int limit) {
  QSqlQuery q = query("logs",
                      "SELECT id, datetime, log, is_err, is_warn FROM logs WHERE datetime >= ? "
                      "ORDER BY datetime DESC LIMIT ? OFFSET ?",
                      {cutoff_iso(), limit, offset});
  return to_rows(q);
}

bool Data::update_logs(qlonglong id, const QString& text, bool is_err, bool is_warn) {
  QSqlQuery q = query("logs", "UPDATE logs SET log = ?, is_err = ?, is_warn = ? WHERE id = ?",
                      {text, is_err, is_warn, id});
  return q.isActive() && q.numRowsAffected() > 0;
}

bool Data::delete_logs(qlonglong id) {
  QSqlQuery q = query("logs", "DELETE FROM logs WHERE id = ?", {id});
  return q.isActive() && q.numRowsAffected() > 0;
}

int Data::count_logs() {
  QSqlQuery q = query("logs", "SELECT COUNT(*) FROM logs WHERE datetime >= ?", {cutoff_iso()});
  return q.next() ? q.value(0).toInt() : 0;
}

int Data::count_error_logs() {
  QSqlQuery q = query("logs", "SELECT COUNT(*) FROM logs WHERE datetime >= ? AND is_err = 1", {cutoff_iso()});
  return q.next() ? q.value(0).toInt() : 0;
}

void Data::purge_logs() { query("logs", "DELETE FROM logs WHERE datetime < ?", {cutoff_iso()}); }

// settings

QVariantMap Data::read_settings() {
  QSqlQuery q = query("settings", "SELECT key, value FROM settings");
  QVariantMap settings;
  while (q.next()) {
    settings.insert(q.value(0).toString(), decode(q.value(1).toString()));
  }
  return settings;
}

bool Data::create_setting(const QString& key, const QVariant& value) {
  return query("settings", "INSERT INTO settings (key, value) VALUES (?, ?)", {key, encode(value)}).isActive();
}

bool Data::update_setting(const QString& key, const QVariant& value) {
  QSqlQuery q = query("settings", "UPDATE settings SET value = ? WHERE key = ?", {encode(value), key});
  return q.isActive() && q.numRowsAffected() > 0;
}

bool Data::delete_setting(const QString& key) {
  QSqlQuery q = query("settings", "DELETE FROM settings WHERE key = ?", {key});
  return q.isActive() && q.numRowsAffected() > 0;
}

// dictionary and snippets share one shape, so they share the pair helpers.

QVariantMap Data::read_dictionary() { return read_pairs("dictionary"); }

bool Data::create_dictionary(const QString& from_text, const QString& to_text) {
  return create_pair("dictionary", from_text, to_text);
}

bool Data::update_dictionary(const QString& from_text, const QString& to_text) {
  return update_pair("dictionary", from_text, to_text);
}

bool Data::delete_dictionary(const QString& from_text) { return delete_pair("dictionary", from_text); }

QVariantMap Data::read_snippets() { return read_pairs("snippets"); }

bool Data::create_snippets(const QString& from_text, const QString& to_text) {
  return create_pair("snippets", from_text, to_text);
}

bool Data::update_snippets(const QString& from_text, const QString& to_text) {
  return update_pair("snippets", from_text, to_text);
}

bool Data::delete_snippets(const QString& from_text) { return delete_pair("snippets", from_text); }

// Table names come from the constants above, never from user input; values are bound.

QVariantMap Data::read_pairs(const char* table) {
  QSqlQuery q = query(table, QStringLiteral("SELECT from_text, to_text FROM %1 ORDER BY from_text").arg(QString::fromLatin1(table)));
  QVariantMap pairs;
  while (q.next()) {
    pairs.insert(q.value(0).toString(), q.value(1).toString());
  }
  return pairs;
}

bool Data::create_pair(const char* table, const QString& from_text, const QString& to_text) {
  return query(table, QStringLiteral("INSERT INTO %1 (from_text, to_text) VALUES (?, ?)").arg(QString::fromLatin1(table)),
               {from_text, to_text})
      .isActive();
}

bool Data::update_pair(const char* table, const QString& from_text, const QString& to_text) {
  QSqlQuery q = query(table, QStringLiteral("UPDATE %1 SET to_text = ? WHERE from_text = ?").arg(QString::fromLatin1(table)),
                      {to_text, from_text});
  return q.isActive() && q.numRowsAffected() > 0;
}

bool Data::delete_pair(const char* table, const QString& from_text) {
  QSqlQuery q = query(table, QStringLiteral("DELETE FROM %1 WHERE from_text = ?").arg(QString::fromLatin1(table)), {from_text});
  return q.isActive() && q.numRowsAffected() > 0;
}

// Runs one statement with bound values. A failure is logged, and raised once per table.
QSqlQuery Data::query(const char* table, const QString& sql, const QVariantList& args) {
  if (!m_open) {  // late log lines during shutdown land here: inactive result, nothing raised
    return QSqlQuery();
  }
  QSqlQuery q(QSqlDatabase::database(CONNECTION));
  q.prepare(sql);
  for (const QVariant& arg : args) {
    q.addBindValue(arg);
  }
  if (!q.exec()) {
    fail(table, q.lastError().text());
  }
  return q;
}

void Data::fail(const char* table, const QString& error) {
  qCWarning(lcData) << table << error;
  const QString key = QLatin1String(table);
  if (!m_failed.contains(key)) {
    m_failed.insert(key);
    emit STORAGE_FAILED(QStringLiteral("%1: %2").arg(key, error));
  }
}
