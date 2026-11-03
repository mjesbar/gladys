// Data - Singleton owner of history.db (SQLite, plaintext). ORM-style CRUD per table, on its own thread.

#ifndef DATA_DATA_HPP
#define DATA_DATA_HPP

#include <QObject>
#include <QSet>
#include <QString>
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>

class QSqlQuery;
class QTimer;

class Data : public QObject {
  Q_OBJECT

 public:
  static Data* create();    // once, on the GUI thread; the caller moves it to its thread
  static Data* instance();  // nullptr before create() and after destruction
  static void post_log(const QString& text, bool is_err, bool is_warn);  // async, any thread
  static void post_transcription(const QString& text);                   // async, any thread

  Data(const Data&) = delete;
  Data& operator=(const Data&) = delete;
  ~Data() override;

 public slots:
  bool open();
  void close();
  void set_toggles(bool store_transcriptions, bool store_logs);
  void push_transcription(const QString& text);  // stored only when Store Transcriptions is on
  void push_log(const QString& text, bool is_err, bool is_warn);  // stored only when Store Logs is on

  // history: id-keyed
  qlonglong create_history(const QString& transcription);
  QVariantList read_history(int offset, int limit);
  bool update_history(qlonglong id, const QString& transcription);
  bool delete_history(qlonglong id);

  // logs: id-keyed; read and count cover the last 30 days
  qlonglong create_logs(const QString& text, bool is_err, bool is_warn);
  QVariantList read_logs(int offset, int limit);
  bool update_logs(qlonglong id, const QString& text, bool is_err, bool is_warn);
  bool delete_logs(qlonglong id);
  int count_logs();
  int count_error_logs();

  // settings: key/value, keyed by key
  QVariantMap read_settings();
  bool create_setting(const QString& key, const QVariant& value);
  bool update_setting(const QString& key, const QVariant& value);
  bool delete_setting(const QString& key);

  // dictionary: from_text -> to_text, keyed by from_text
  QVariantMap read_dictionary();
  bool create_dictionary(const QString& from_text, const QString& to_text);
  bool update_dictionary(const QString& from_text, const QString& to_text);
  bool delete_dictionary(const QString& from_text);

  // snippets: from_text -> to_text, keyed by from_text
  QVariantMap read_snippets();
  bool create_snippets(const QString& from_text, const QString& to_text);
  bool update_snippets(const QString& from_text, const QString& to_text);
  bool delete_snippets(const QString& from_text);

 signals:
  void STORAGE_FAILED(const QString& what);  // raised once per table per session

 private:
  Data() = default;

  QSqlQuery query(const char* table, const QString& sql, const QVariantList& args = {});
  void fail(const char* table, const QString& error);
  void purge_logs();
  QVariantMap read_pairs(const char* table);
  bool create_pair(const char* table, const QString& from_text, const QString& to_text);
  bool update_pair(const char* table, const QString& from_text, const QString& to_text);
  bool delete_pair(const char* table, const QString& from_text);

  bool m_store_transcriptions = true;
  bool m_store_logs = true;
  QSet<QString> m_failed;
  QTimer* m_purge = nullptr;
  bool m_open = false;  // false before open() and after close(): writes are dropped, not raised
};

#endif  // DATA_DATA_HPP
