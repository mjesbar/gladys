// LLM Module - Manages llama-server (QProcess) and formats transcriptions over HTTP.

#ifndef LLM_H
#define LLM_H

#include <QNetworkAccessManager>
#include <QObject>
#include <QProcess>
#include <QString>

class LLM : public QObject {
  Q_OBJECT

 public:
  explicit LLM(QObject* parent = nullptr);
  ~LLM() override;

  bool start();
  void stop();

 public slots:
  void format_transcription(const QString& text);

 signals:
  void FORMATTED_TEXT_READY(const QString& text);

 private:
  QProcess m_server;
  QNetworkAccessManager m_network;
  bool m_is_running;
};

#endif  // LLM_H
