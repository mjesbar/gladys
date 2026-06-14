// LLM Module - llama.cpp server for audio transcription.

#ifndef LLM_H
#define LLM_H

#include <QObject>
#include <QProcess>
#include <QString>
#include <QByteArray>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#ifdef Q_OS_WIN
#define LLAMA_SERVER_BINARY "llama-server.exe"
#else
#define LLAMA_SERVER_BINARY "llama-server"
#endif

class LLM : public QObject {
  Q_OBJECT

public:
  LLM(const LLM &) = delete;
  LLM &operator=(const LLM &) = delete;

  static LLM *instance();
  static bool start();
  static void stop();
  static QString formatTranscription(const QString &text);

private:
  explicit LLM(QObject *parent = nullptr);
  ~LLM() override;

  static LLM *m_instance;
  static QProcess *m_serverProcess;
  static QString m_serverUrl;
  static bool m_isRunning;

  static QString postJson(const QJsonObject &json);
};

#endif // LLM_H