// LLM - llama-server launch and chat request/reply for transcription formatting. Core spawns and
// supervises it.

#include "llm.hpp"

#include <QCoreApplication>
#include <QJsonArray>

#include "../../modules/aimodmgr/aimodmgr.hpp"
#include "../server.hpp"
#include "prompt.hpp"

#ifdef Q_OS_WIN
#define LLAMA_SERVER_BINARY "llama-server.exe"
#else
#define LLAMA_SERVER_BINARY "llama-server"
#endif

static QString llama_dir() {
  return QCoreApplication::applicationDirPath() + "/llama.cpp/llama-b9265";
}

QString llama_server_program() { return llama_dir() + "/" + LLAMA_SERVER_BINARY; }

QStringList llama_server_args() {
  return QStringList() << "--model" << selected_model_path(ModelKind::Llm) << "--ctx-size" << "4096"
                       << "--temperature" << "0.7"
                       << "--top-p" << "0.9"
                       << "--reasoning" << "off"
                       << "--reasoning-budget" << "0"
                       << "--n-gpu-layers" << "16"
                       << "--flash-attn" << "on"
                       << "--cache-type-k" << "q4_0"
                       << "--repeat-penalty" << "1.15"
                       << "--repeat-last-n" << "16"
                       << "--port" << QString::number(LLM_PORT);
}

QProcessEnvironment llama_server_env() {
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert("LD_LIBRARY_PATH", llama_dir() + ":" + env.value("LD_LIBRARY_PATH"));
  return env;
}

// Literal, deterministic replacements applied to the LLM output.
static QString apply_snippets(QString text, const QVariantMap& snippets) {
  for (auto it = snippets.cbegin(); it != snippets.cend(); ++it) {
    text.replace(it.key(), it.value().toString());
  }
  return text;
}

QJsonObject build_format_request(const QString& text, const QVariantMap& settings) {
  QJsonObject system;
  system["role"] = "system";
  system["content"] = build_system_prompt(settings.value("style").toString(),
    settings.value("markdown").toBool(),
    settings.value("dictionary").toMap());

  QJsonObject user;
  user["role"] = "user";
  user["content"] =
    "TASK\nFormat the input according to the instructions & examples above.\n"
    "input: " +
    text + "\noutput: ";

  QJsonObject json;
  json["messages"] = QJsonArray({system, user});
  json["stop"] = QJsonArray({"\ninput:", "\n\n", "\n \n"});
  json["max_tokens"] = static_cast<int>(text.length() + 10);
  return json;
}

// An empty reply (request failed or malformed) yields the raw text, so the user never loses their
// dictation.
QString read_format_reply(
  const QJsonObject& reply, const QString& raw, const QVariantMap& snippets) {
  const QJsonArray choices = reply["choices"].toArray();
  const QString content = choices.isEmpty()
    ? QString()
    : choices[0].toObject()["message"].toObject()["content"].toString();
  const QString result = apply_snippets(content, snippets);
  return result.isEmpty() ? raw : result;
}
