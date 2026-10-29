// Core - Pipeline and supervisor for gladys-asr and gladys-llm. Runs on the GUI thread.
// Flow: hotkey -> asr /transcribe -> llm /format -> clipboard -> paste.
// Talks to the APIs over HTTP JSON on loopback; restarts an API after a crash.

#ifndef CORE_HPP
#define CORE_HPP

#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QObject>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QVariantMap>
#include <functional>

class Paste;

class Core : public QObject {
  Q_OBJECT

 public:
  Core(const QString& model_dir, Paste* paste, QObject* parent = nullptr);
  ~Core() override;

  bool start();  // spawns gladys-asr and gladys-llm; READY once both answer /alive

 public slots:
  void toggle_recording();
  void update_config(const QVariantMap& config);

 signals:
  void READY();
  void FAILED(const QString& reason);
  void RECORDING_CHANGED(bool recording);

 private:
  struct Child {
    QString name;
    QString program;
    QStringList args;
    QString url;  // API base URL, e.g. http://127.0.0.1:8081
    QProcess process;
    bool ready = false;
  };

  void spawn(Child& child);
  void on_exit(Child& child);
  void probe(Child& child);  // GET /alive
  void check_ready();
  void post(const QString& url, const QJsonObject& body, std::function<void(const QJsonObject&)> done);
  void format_text(const QString& text);
  void paste_text(const QString& text);

  Paste* m_paste;
  Child m_asr;
  Child m_llm;
  QNetworkAccessManager m_network;
  QTimer m_probe_timer;
  QVariantMap m_config;
  bool m_recording = false;
  bool m_stopping = false;
};

#endif  // CORE_HPP
