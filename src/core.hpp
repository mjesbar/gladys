// Core - Pipeline and supervisor for gladys-asr and gladys-llm (llama-server). Runs on the GUI thread.
// Flow: hotkey -> asr /transcribe -> llama-server /v1/chat/completions -> clipboard -> paste.
// Talks to the APIs over HTTP JSON on loopback; restarts a child after a crash.

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

  bool start();  // spawns gladys-asr and gladys-llm; READY once both are ready

 public slots:
  void hotkey_down();  // Toggle flips recording; Hold starts it
  void hotkey_up();    // Hold stops it
  void update_config(const QVariantMap& config);

 signals:
  void READY();
  void FAILED(const QString& reason);
  void RECORDING_CHANGED(bool recording);
  void PROCESSING_CHANGED(bool processing);

 private:
  struct Child {
    QString name;
    QString program;
    QStringList args;
    QString url;  // API base URL, e.g. http://127.0.0.1:26000
    QString ready_path = "/alive";  // GET returns 200 once ready
    QProcess process;
    QByteArray err_buf;  // stderr bytes not yet ending in a newline
    bool ready = false;
  };

  void spawn(Child& child);
  void on_exit(Child& child);
  void read_stderr(Child& child);  // echoes each line and stores it in logs
  void probe(Child& child);  // GET ready_path
  void check_ready();
  void post(const QString& url, const QJsonObject& body, std::function<void(const QJsonObject&)> done);
  void start_recording();
  void stop_recording();
  void format_text(const QString& text);
  void paste_text(const QString& text);
  void add_pending(int delta);  // dictations between hotkey stop and paste

  Paste* m_paste;
  Child m_asr;
  Child m_llm;
  QNetworkAccessManager m_network;
  QTimer m_probe_timer;
  QVariantMap m_config;
  int m_pending = 0;
  bool m_recording = false;
  bool m_hold_mode = false;  // hotkeyMode is Hold
  bool m_stopping = false;
};

#endif  // CORE_HPP
