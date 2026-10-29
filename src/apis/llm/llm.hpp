// LLM - Owns llama-server (QProcess) and formats transcriptions. Runs inside the gladys-llm process.

#ifndef LLM_H
#define LLM_H

#include <QNetworkAccessManager>
#include <QObject>
#include <QProcess>
#include <QString>
#include <QTimer>
#include <QVariantMap>
#include <functional>

class LLM : public QObject {
  Q_OBJECT

 public:
  using Done = std::function<void(const QString& text)>;

  explicit LLM(QObject* parent = nullptr);
  ~LLM() override;

  bool start();  // spawns llama-server; is_ready() turns true once /health answers 200
  void stop();
  bool is_ready() const;

  // settings: style, markdown, dictionary, snippets. done gets the formatted text,
  // or the raw text when the request fails.
  void format(const QString& text, const QVariantMap& settings, Done done);

 private:
  void poll_health();

  QProcess m_server;
  QNetworkAccessManager m_network;
  QTimer m_health_timer;
  bool m_is_ready = false;
};

#endif  // LLM_H
