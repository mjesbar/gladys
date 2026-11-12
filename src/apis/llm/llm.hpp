// LLM - llama-server launch and chat request/reply for transcription formatting. Core spawns and supervises it.

#ifndef LLM_H
#define LLM_H

#include <QJsonObject>
#include <QProcessEnvironment>
#include <QString>
#include <QStringList>
#include <QVariantMap>

QString llama_server_program();  // llama-server next to the app
QStringList llama_server_args();  // model, sampling, and --port LLM_PORT
QProcessEnvironment llama_server_env();

// settings: style, markdown, dictionary. Body for POST /v1/chat/completions.
QJsonObject build_format_request(const QString& text, const QVariantMap& settings);

// Formatted text from the chat reply, with snippets applied. Falls back to raw on any failure.
QString read_format_reply(const QJsonObject& reply, const QString& raw, const QVariantMap& snippets);

#endif  // LLM_H
