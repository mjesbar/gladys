// LLM Module - llama.cpp server for audio transcription.

#include "llm.h"
#include "gladysd.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QTimer>
#include <iostream>

LLM *LLM::m_instance = nullptr;
QProcess *LLM::m_serverProcess = nullptr;
QString LLM::m_serverUrl = "http://127.0.0.1:8080";
bool LLM::m_isRunning = false;

LLM::LLM(QObject *parent) : QObject(parent) {}

LLM::~LLM() {
  stop();
}

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
QString llama_path = exe_path + "/llama.cpp/llama-b9049-vulkan";
QString program = llama_path + "/llama-server";

QFile file(program);
if (!file.exists()) {
  std::cerr << "LLM: llama-server not found at: " << qPrintable(program) << std::endl;
  return false;
}

m_serverProcess = new QProcess();
m_serverProcess->setProgram(program);
m_serverProcess->setArguments(QStringList()
  << "-m" << exe_path + "/llama.cpp/llama-b9049-vulkan/models/Llama-3.2-1B-Instruct-Q4_K_M.gguf"
  << "--mmproj" << exe_path + "/llama.cpp/llama-b9049-vulkan/models/mmproj-ultravox-v0_5-llama-3_2-1b-f16.gguf"
  << "--ctx-size" << "8192"
  << "--n-gpu-layers" << "99"
  << "-ngl" << "99"
  << "--port" << "8080");

QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
env.insert("LD_LIBRARY_PATH", llama_path + ":" + env.value("LD_LIBRARY_PATH"));
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
  std::cout << "LLM: Server started on " << qPrintable(m_serverUrl) << std::endl;
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

QString LLM::transcript(const QString &audioPath) {
  if (!m_isRunning) {
    std::cerr << "LLM: Server not running." << std::endl;
    return QString();
  }

  QFile file(audioPath);
  if (!file.open(QIODevice::ReadOnly)) {
    std::cerr << "LLM: Cannot open audio file: " << qPrintable(audioPath) << std::endl;
    return QString();
  }
  QByteArray audioData = file.readAll().toBase64();
  file.close();

  return transcriptFromMemory(QByteArray::fromBase64(audioData));
}

static QByteArray createWavHeader(int sampleRate, int channels, int bitsPerSample, int dataSize) {
  QByteArray header;
  header.reserve(44);

  // RIFF header
  header.append("RIFF");
  int fileSize = 36 + dataSize;
  header.append(reinterpret_cast<const char *>(&fileSize), 4);
  header.append("WAVE");

  // fmt chunk
  header.append("fmt ");
  int fmtSize = 16;
  header.append(reinterpret_cast<const char *>(&fmtSize), 4);
  short audioFormat = 1; // PCM
  header.append(reinterpret_cast<const char *>(&audioFormat), 2);
  short numChannels = channels;
  header.append(reinterpret_cast<const char *>(&numChannels), 2);
  int sr = sampleRate;
  header.append(reinterpret_cast<const char *>(&sr), 4);
  int byteRate = sampleRate * channels * bitsPerSample / 8;
  header.append(reinterpret_cast<const char *>(&byteRate), 4);
  short blockAlign = channels * bitsPerSample / 8;
  header.append(reinterpret_cast<const char *>(&blockAlign), 2);
  short bps = bitsPerSample;
  header.append(reinterpret_cast<const char *>(&bps), 2);

  // data chunk
  header.append("data");
  header.append(reinterpret_cast<const char *>(&dataSize), 4);

  return header;
}

QString LLM::transcriptFromMemory(const QByteArray &pcmData) {
  if (!m_isRunning) {
    std::cerr << "LLM: Server not running." << std::endl;
    return QString();
  }

  if (pcmData.isEmpty()) {
    std::cerr << "LLM: No audio data to transcribe." << std::endl;
    return QString();
  }

  // Create WAV header: 16kHz, mono, 16-bit
  const int sampleRate = 16000;
  const int channels = 1;
  const int bitsPerSample = 16;
  QByteArray wavHeader = createWavHeader(sampleRate, channels, bitsPerSample, pcmData.size());

  // Combine header + PCM data
  QByteArray wavData = wavHeader + pcmData;
  QByteArray audioBase64 = wavData.toBase64();

  QJsonObject message;
  message["role"] = "user";

  // Llama Ultravox format: content as array with type fields
  QJsonArray content;

  QJsonObject audioObj;
  audioObj["type"] = "input_audio";
  audioObj["input_audio"] = QJsonObject({{"data", QString::fromUtf8(audioBase64)}, {"format", "wav"}});
  content.append(audioObj);

  QJsonObject textObj;
  textObj["type"] = "text";
  textObj["text"] = "Transcribe the audio. Output ONLY the transcribed text.";
  content.append(textObj);

  message["content"] = content;

  QJsonObject json;
  json["messages"] = QJsonArray({message});
  json["temperature"] = 0.0;

  std::cout << "LLM: Sending audio to server... (" << audioBase64.size() << " base64 bytes)" << std::endl;
  QString response = postJson(json);
  std::cout << "LLM: Received response: " << response.left(200).toStdString() << "..." << std::endl;

  QJsonDocument doc = QJsonDocument::fromJson(response.toUtf8());
  if (!doc.isNull() && doc.isObject()) {
    QJsonObject obj = doc.object();
    QJsonArray choices = obj["choices"].toArray();
    if (!choices.isEmpty()) {
      QString result = choices[0].toObject()["message"].toObject()["content"].toString();
      std::cout << "LLM: Transcript: " << qPrintable(result) << std::endl;
      return result;
    }
  }

  std::cerr << "LLM: Failed to parse response." << std::endl;
  return QString();
}