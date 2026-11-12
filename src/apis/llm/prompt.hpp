// Prompt - Builds the system prompt that drives the transcription formatter.

#ifndef LLM_PROMPT_HPP
#define LLM_PROMPT_HPP

#include <QString>
#include <QVariantMap>

// dictionary: rare word -> brief description, kept exactly as written.
QString build_system_prompt(const QString& style, bool markdown, const QVariantMap& dictionary);

#endif  // LLM_PROMPT_HPP
