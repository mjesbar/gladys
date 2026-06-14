// LLM Module - llama.cpp server for audio transcription.

#include "llm.h"
#include "gladysd.h"
#include "settings/settings_manager.h"
#include <QCoreApplication>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QProcess>
#include <QTimer>
#include <iostream>

LLM *LLM::m_instance = nullptr;
QProcess *LLM::m_serverProcess = nullptr;
QString LLM::m_serverUrl = "http://127.0.0.1:8080";
bool LLM::m_isRunning = false;

LLM::LLM(QObject *parent) : QObject(parent) {}

LLM::~LLM() { stop(); }

LLM *LLM::instance() {
  if (!m_instance) {
    m_instance = new LLM();
  }
  return m_instance;
}

bool LLM::start() {
  if (m_isRunning) {
    std::cout << "LLM: Server already running." << std::endl;
    return true;
  }

  QString exe_path = QCoreApplication::applicationDirPath();
  QString llama_path = exe_path + "/llama.cpp/llama-b9265";
  QString program = llama_path + "/" + LLAMA_SERVER_BINARY;

  QFile file(program);
  if (!file.exists()) {
    std::cerr << "LLM: llama-server not found at: " << qPrintable(program)
              << std::endl;
    return false;
  }

  m_serverProcess = new QProcess();
  m_serverProcess->setProgram(program);
  m_serverProcess->setArguments(
      QStringList() << "-m"
                    << exe_path + "/llama.cpp/llama-b9265/models/"
                                  "Llama-3.2-1B-Instruct-Q4_K_M.gguf"
                    << "--ctx-size" << "8192"
                    << "--n-gpu-layers" << "99"
                    << "-ngl" << "99"
                    << "--port" << "8080");

  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert("LD_LIBRARY_PATH",
             llama_path + ":" + env.value("LD_LIBRARY_PATH"));
  m_serverProcess->setProcessEnvironment(env);

  QObject::connect(m_serverProcess, &QProcess::readyReadStandardError, []() {
    // Optionally log server output for debugging
  });

  m_serverProcess->start();

  if (!m_serverProcess->waitForStarted(5000)) {
    std::cerr << "LLM: Failed to start llama-server." << std::endl;
    delete m_serverProcess;
    m_serverProcess = nullptr;
    return false;
  }

  // Wait for server to be ready
  QEventLoop loop;
  QTimer timeout;
  timeout.setSingleShot(true);
  QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
  timeout.start(10000);
  QTimer::singleShot(2000, &loop, &QEventLoop::quit);
  loop.exec();

  m_isRunning = true;
  std::cout << "LLM: Server started on " << qPrintable(m_serverUrl)
            << std::endl;
  return true;
}

void LLM::stop() {
  if (m_serverProcess) {
    m_serverProcess->terminate();
    if (!m_serverProcess->waitForFinished(3000)) {
      m_serverProcess->kill();
    }
    delete m_serverProcess;
    m_serverProcess = nullptr;
  }
  m_isRunning = false;
  std::cout << "LLM: Server stopped." << std::endl;
}

QString LLM::postJson(const QJsonObject &json) {
  QNetworkAccessManager manager;
  QNetworkRequest request;
  request.setUrl(QUrl(m_serverUrl + "/v1/chat/completions"));
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

  QJsonDocument doc(json);
  QByteArray postData = doc.toJson();

  QNetworkReply *reply = manager.post(request, postData);

  // Wait for response
  QEventLoop loop;
  QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
  loop.exec();

  QByteArray response = reply->readAll();
  reply->deleteLater();

  return QString::fromUtf8(response);
}

QString LLM::formatTranscription(const QString &text) {
  if (!m_isRunning) {
    std::cerr << "LLM: Server not running." << std::endl;
    return text;
  }

  if (text.isEmpty()) {
    std::cerr << "LLM: No text to beautify." << std::endl;
    return text;
  }

  SettingsManager *s = SettingsManager::instance();
  QStringList rules;
  int ruleNum = 1;

  // Always apply proper casing
  rules << QString("%1. Apply proper casing: capitalize the first word of "
                   "each sentence and proper nouns.")
               .arg(ruleNum++);

  // Punctuation
  if (s->allowPunctuation()) {
    rules << QString("%1. Add proper punctuation (periods, commas, question "
                     "marks, etc.) where grammatically required.")
                 .arg(ruleNum++);
  } else {
    rules << QString("%1. Do NOT add any punctuation. Output without periods, "
                     "commas, or any punctuation marks.")
                 .arg(ruleNum++);
  }

  // Accents
  if (!s->allowAccents()) {
    rules << QString("%1. Remove all accents and diacritical marks from "
                     "characters (e.g. é -> e, ñ -> n).")
                 .arg(ruleNum++);
  }

  // Markdown
  if (!s->markdownMode()) {
    rules << QString("%1. NEVER use Markdown, bullet points, bold, italics, "
                     "headers, code blocks, or any formatting syntax. Output "
                     "plain text only.")
                 .arg(ruleNum++);
  }

  // Dictionary aliases
  const auto dict = s->dictionary();
  for (auto it = dict.cbegin(); it != dict.cend(); ++it) {
    rules << QString("%1. Replace the word \"%2\" with \"%3\".")
                 .arg(ruleNum++)
                 .arg(it.key(), it.value());
  }

  // Always preserve exact content
  rules << QString(
      "%1. Do not add, remove, or rephrase any words. Preserve the exact "
      "spoken content.")
               .arg(ruleNum++);
  rules << QString(
      "%1. Return ONLY the formatted transcription. No greetings, no "
      "explanations, no commentary.")
               .arg(ruleNum++);

  QString prompt =
      "You are a speech transcription formatter. Your sole task is to take "
      "raw speech-to-text output and produce a clean, readable transcription. "
      "Strict rules:\n" +
      rules.join("\n") + "\n\nRaw text:\n" + text;

  QJsonObject message;
  message["role"] = "user";
  message["content"] = prompt;

  QJsonObject json;
  json["messages"] = QJsonArray({message});
  json["temperature"] = 0.0;

  std::cout << "LLM: Beautifying text: " << text.left(50).toStdString() << "..."
            << std::endl;
  QString response = postJson(json);
  std::cout << "LLM: Beautified response: " << response.left(200).toStdString()
            << "..." << std::endl;

  QJsonDocument doc = QJsonDocument::fromJson(response.toUtf8());
  if (!doc.isNull() && doc.isObject()) {
    QJsonObject obj = doc.object();
    QJsonArray choices = obj["choices"].toArray();
    if (!choices.isEmpty()) {
      QString result =
          choices[0].toObject()["message"].toObject()["content"].toString();
      std::cout << "LLM: Beautified: " << qPrintable(result) << std::endl;
      return result;
    }
  }

  std::cerr << "LLM: Failed to parse beautify response." << std::endl;
  return text;
}
