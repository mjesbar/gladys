// LLM Module - Manages llama-server (QProcess) and formats transcriptions over
// HTTP.

#include "llm.hpp"

#include <qt6/QtCore/qcoreapplication.h>

#include <QCoreApplication>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QThread>

#include "../log/log.hpp"
#include "../setting/setting.hpp"

#ifdef Q_OS_WIN
#define LLAMA_SERVER_BINARY "llama-server.exe"
#else
#define LLAMA_SERVER_BINARY "llama-server"
#endif

static const char* SERVER_URL = "http://127.0.0.1:8080/v1/chat/completions";

static QString build_prompt(const QString& text) {
  Setting* s = Setting::instance();
  const QString style = s->style();
  const bool markdown = s->markdown_mode();

  QString instructions =
    "INSTRUCTIONS\n"
    "You are a speech transcription formatter. Your sole task is to take raw "
    "speech-to-text output and produce a clean, readable transcription.\n"
    "- Apply proper casing: capitalize the first word of each sentence and "
    "proper nouns.\n"
    "- Do not add, remove, or rephrase any words. Preserve the exact spoken "
    "content.\n"
    "- Return ONLY the formatted transcription. No greetings, no explanations, "
    "no commentary.\n"
    "- Keep the fidelity of the intention within the sentence, so avoid complex "
    "rephrasing, reorganizations or restructurations.\n"
    "- Do not modify personal names. Keep them exactly as spoken.\n"
    "- Do not repeat a word or phrase unless it was repeated in the speech.\n"
    "- Remove consecutive duplicated words caused by speech stutter, e.g. \"this one "
    "on on the table\" -> \"this one on the table\".\n"
    "---\n"
    "CONFIGURATION\n"
    "- Style: " +
    style + "\n";

  if (style == "Casual") {
    instructions += "- Use a relaxed, conversational tone. Keep it informal.\n";
  } else if (style == "Formal") {
    instructions += "- Use a formal, professional tone with complete sentences.\n";
  } else {
    instructions += "- Format as an email with a polite, professional tone.\n";
  }

  instructions += markdown
    ? "- Markdown: Yes. When the speech lists items, output them as bullet points "
      "and still transcribe the surrounding meaning as full sentences.\n"
    : "- Markdown: No. Output plain text only, no formatting syntax.\n";

  QString style_examples;
  if (style == "Casual") {
    style_examples =
      "Input: hey so um i was thinking we could maybe grab lunch later today if you are free and "
      "the weather is nice that would be really fun\n"
      "Output: Hey, so um, I was thinking we could maybe grab lunch later today if you are free. "
      "If the weather is nice, that would be really fun!\n"
      "Input: yeah that sounds good i will see you there around noon and we can figure out the "
      "rest when we get to the place\n"
      "Output: Yeah, that sounds good. I will see you there around noon, and we can figure out the "
      "rest when we get to the place.\n"
      "Input: dont forget to bring the snacks for the game tonight because last time we ran out "
      "before the second half even started\n"
      "Output: Don't forget to bring the snacks for the game tonight, because last time we ran out "
      "before the second half even started!\n"
      "Input: lol that was honestly the best movie ever i cant believe the ending and i already "
      "want to watch it again this weekend\n"
      "Output: Lol, that was honestly the best movie ever. I can't believe the ending, and I "
      "already want to watch it again this weekend.\n"
      "Input: i cant make it tonight sorry about that but i will be free tomorrow after work if "
      "you want to do something then instead\n"
      "Output: I can't make it tonight, sorry about that. I will be free tomorrow after work, if "
      "you want to do something then instead.\n"
      "Input: i think we should probably move the meeting to thursday\n"
      "Output: I think we should probably move the meeting to Thursday.\n";
  } else if (style == "Formal") {
    style_examples =
      "Input: i would like to request a meeting next week if possible to discuss the revised "
      "timeline and the budget changes we talked about last month\n"
      "Output: I would like to request a meeting next week, if possible, to discuss the revised "
      "timeline and the budget changes we discussed last month.\n"
      "Input: thank you for your time and consideration regarding this matter we really appreciate "
      "you taking the time to review our proposal in detail\n"
      "Output: Thank you for your time and consideration regarding this matter. We really "
      "appreciate you taking the time to review our proposal in detail.\n"
      "Input: we cannot proceed with the next phase until the budget is approved by the finance "
      "committee and the signed contract is returned to our office\n"
      "Output: We cannot proceed with the next phase until the budget is approved by the finance "
      "committee and the signed contract is returned to our office.\n"
      "Input: please find the attached document for your review it contains the full "
      "specifications along with the revised delivery schedule and pricing\n"
      "Output: Please find the attached document for your review. It contains the full "
      "specifications, along with the revised delivery schedule and pricing.\n"
      "Input: i look forward to hearing from you soon and please let me know if you require any "
      "additional information from my side to move things forward\n"
      "Output: I look forward to hearing from you soon. Please let me know if you require any "
      "additional information from my side to move things forward.\n"
      "Input: i think we should probably move the meeting to thursday\n"
      "Output: I think we should probably move the meeting to Thursday.\n";
  } else {
    style_examples =
      "Input: can you send the invoice by friday thanks a lot it would really help the accounting "
      "team close the books on time this month\n"
      "Output: Could you please send the invoice by Friday? Thank you very much. It would really "
      "help the accounting team close the books on time this month.\n"
      "Input: sorry i missed your call earlier let me know when you are free again because i want "
      "to go over the numbers before the client meeting\n"
      "Output: Sorry I missed your call earlier. Please let me know when you are free again, "
      "because I want to go over the numbers before the client meeting.\n"
      "Input: the deadline moved to monday so we have more time to polish the final version and "
      "make sure every section is reviewed properly\n"
      "Output: The deadline has moved to Monday, so we have more time to polish the final version. "
      "I want to make sure every section is reviewed properly.\n"
      "Input: i attached the slides for tomorrow please review them when you can and send me any "
      "comments you have on the second and third sections\n"
      "Output: I have attached the slides for tomorrow. Please review them when you can, and send "
      "me any comments you have on the second and third sections.\n"
      "Input: happy to join the call at ten that works for me and i will prepare a short summary "
      "of the open items so we can move quickly through them\n"
      "Output: Happy to join the call at ten, and that works for me. I will prepare a short "
      "summary of the open items so we can move quickly through them.\n"
      "Input: i think we should probably move the meeting to thursday\n"
      "Output: I think we should probably move the meeting to Thursday.\n";
  }

  QString markdown_examples;
  if (markdown) {
    markdown_examples =
      "Input: the shopping list for the weekend is milk eggs bread and some fresh apples because "
      "the kids want to bake a cake on sunday\n"
      "Output: The shopping list for the weekend includes:\n- Milk\n- Eggs\n- Bread\n- Fresh "
      "apples\nThe kids want to bake a cake on Sunday.\n"
      "Input: this is really important so please do not forget it because the whole project "
      "depends on the final approval being signed on time\n"
      "Output: **This is really important**, so please do not forget it. The whole project depends "
      "on the final approval being signed on time.\n"
      "Input: first call the vendor to confirm the delivery date then update the budget sheet and "
      "finally send the summary to the team leads\n"
      "Output: The steps are:\n1. Call the vendor to confirm the delivery date\n2. Update the "
      "budget sheet\n3. Send the summary to the team leads\n"
      "Input: the groceries we still need are apples rice coffee and a few lemons so we can finish "
      "the recipes planned for the dinner party\n"
      "Output: We still need these groceries for the dinner party:\n- Apples\n- Rice\n- Coffee\n- "
      "Lemons\n"
      "Input: this part matters most so read it twice before the meeting because the decisions "
      "made there will shape the entire next quarter\n"
      "Output: **This part matters most**, so read it twice before the meeting. The decisions made "
      "there will shape the entire next quarter.\n";
  } else {
    markdown_examples =
      "Input: the shopping list for the weekend is milk eggs bread and some fresh apples because "
      "the kids want to bake a cake on sunday\n"
      "Output: The shopping list for the weekend is milk, eggs, bread and some fresh apples, "
      "because the kids want to bake a cake on Sunday.\n"
      "Input: this is really important so please do not forget it because the whole project "
      "depends on the final approval being signed on time\n"
      "Output: This is really important, so please do not forget it. The whole project depends on "
      "the final approval being signed on time.\n"
      "Input: first call the vendor to confirm the delivery date then update the budget sheet and "
      "finally send the summary to the team leads\n"
      "Output: First call the vendor to confirm the delivery date, then update the budget sheet, "
      "and finally send the summary to the team leads.\n"
      "Input: the groceries we still need are apples rice coffee and a few lemons so we can finish "
      "the recipes planned for the dinner party\n"
      "Output: The groceries we still need are apples, rice, coffee and a few lemons, so we can "
      "finish the recipes planned for the dinner party.\n"
      "Input: this part matters most so read it twice before the meeting because the decisions "
      "made there will shape the entire next quarter\n"
      "Output: This part matters most, so read it twice before the meeting. The decisions made "
      "there will shape the entire next quarter.\n";
  }

  instructions += "---\nEXAMPLES\n" + style_examples + markdown_examples;

  const auto dict = s->dictionary();
  for (auto it = dict.cbegin(); it != dict.cend(); ++it) {
    instructions += "- Keep the rare word \"" + it.key() +
      "\" exactly as written. Meaning: " + it.value() + ".\n";
  }

  instructions += "---\nTASK\ninput: " + text + "\noutput: ";

  return instructions;
}

// Returns the message content, or an empty string when the reply is malformed.
static QString parse_content(const QByteArray& body) {
  QJsonDocument doc = QJsonDocument::fromJson(body);
  if (!doc.isObject()) {
    return QString();
  }
  QJsonArray choices = doc.object()["choices"].toArray();
  if (choices.isEmpty()) {
    return QString();
  }
  return choices[0].toObject()["message"].toObject()["content"].toString();
}

// Literal, deterministic replacements applied to the LLM output.
static QString apply_snippets(QString text) {
  const auto snippets = Setting::instance()->snippets();
  for (auto it = snippets.cbegin(); it != snippets.cend(); ++it) {
    text.replace(it.key(), it.value());
  }
  return text;
}

LLM::LLM(QObject* parent) : QObject(parent), m_is_running(false) {}

LLM::~LLM() { stop(); }

bool LLM::start() {
  if (m_is_running) {
    return true;
  }

  QString dir = QCoreApplication::applicationDirPath() + "/llama.cpp/llama-b9265";
  QString model = dir + "/models/Llama-3.2-1B-Instruct-Q4_K_M.gguf";
  QString program = dir + "/" + LLAMA_SERVER_BINARY;
  if (!QFileInfo::exists(program)) {
    qCCritical(lcLlm) << "llama-server not found at:" << program;
    return false;
  }

  m_server.setProgram(program);
  m_server.setArguments(QStringList() << "--model" << model << "--ctx-size" << "4096"
                                      << "--temperature" << "0.7"
                                      << "--top-p" << "0.9"
                                      << "--reasoning" << "off"
                                      << "--reasoning-budget" << "0"
                                      << "--n-gpu-layers" << "16"
                                      << "--flash-attn" << "on"
                                      << "--cache-type-k" << "q4_0"
                                      << "--repeat-penalty" << "1.15"
                                      << "--repeat-last-n" << "256"
                                      << "--port" << "8080");

  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert("LD_LIBRARY_PATH", dir + ":" + env.value("LD_LIBRARY_PATH"));
  m_server.setProcessEnvironment(env);

  m_server.start();
  if (!m_server.waitForStarted(5000)) {
    qCCritical(lcLlm) << "failed to start llama-server";
    return false;
  }

  // Startup only (before the UI is shown): give the server time to load the
  // model
  QThread::sleep(2);
  m_is_running = true;
  qCInfo(lcLlm) << "started, server ready";
  return true;
}

void LLM::stop() {
  if (m_server.state() != QProcess::NotRunning) {
    m_server.terminate();
    if (!m_server.waitForFinished(3000)) {
      m_server.kill();
      m_server.waitForFinished(1000);
    }
  }
  m_is_running = false;
  qCInfo(lcLlm) << "stopped";
}

// Async: emits FORMATTED_TEXT_READY when the server replies. On any failure
// the raw text is emitted, so the user never loses their dictation.
void LLM::format_transcription(const QString& text) {
  if (!m_is_running || text.isEmpty()) {
    emit FORMATTED_TEXT_READY(text);
    return;
  }

  qCInfo(lcLlm) << "request:" << text.length() << "chars:" << text;

  QJsonObject message;
  message["role"] = "user";
  message["content"] = build_prompt(text);

  QJsonObject json;
  json["messages"] = QJsonArray({message});
  json["temperature"] = 0.0;

  QNetworkRequest request{QUrl(SERVER_URL)};
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  QNetworkReply* reply = m_network.post(request, QJsonDocument(json).toJson());

  connect(reply, &QNetworkReply::finished, this, [this, reply, text]() {
    reply->deleteLater();
    QString result;
    if (reply->error() == QNetworkReply::NoError) {
      result = apply_snippets(parse_content(reply->readAll()));
    } else {
      qCCritical(lcLlm) << "request failed:" << reply->errorString();
    }
    qCInfo(lcLlm) << "response:" << result.length() << "chars:" << result;
    emit FORMATTED_TEXT_READY(result.isEmpty() ? text : result);
  });
}
