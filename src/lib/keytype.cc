#include "keytype.h"

KeyType *KeyType::s_instance = nullptr;

KeyType::KeyType(QObject *parent) : QObject(parent), m_processing(false) {
  m_process = new QProcess(this);
}

KeyType *KeyType::instance() {
  if (!s_instance) {
    s_instance = new KeyType();
  }
  return s_instance;
}

QString KeyType::extractNewChunk(const QString &newText) {
  // If this is the first text, push it entirely (trimmed)
  if (m_lastTyped.isEmpty()) {
    QString trimmed = newText.trimmed();
    m_lastTyped = newText;
    return trimmed;
  }

  // Find where the new text starts (search from end of last typed)
  int lastLen = m_lastTyped.length();
  if (newText.length() <= lastLen) {
    // Text got shorter or same - ignore (final result or duplicate)
    return QString();
  }

  // Extract only the new portion
  QString diff = newText.mid(lastLen);

  // Clean leading/trailing whitespace
  diff = diff.trimmed();
  if (diff.isEmpty()) {
    return QString();
  }

  // Always prepend a single space
  diff = " " + diff;

  m_lastTyped = newText;
  return diff;
}

QString KeyType::normalizeText(const QString &text) {
  QString result = text;

  // Spanish accented characters
  result.replace("á", "a");
  result.replace("é", "e");
  result.replace("í", "i");
  result.replace("ó", "o");
  result.replace("ú", "u");
  result.replace("Á", "A");
  result.replace("É", "E");
  result.replace("Í", "I");
  result.replace("Ó", "O");
  result.replace("Ú", "U");

  // Spanish special chars
  result.replace("ñ", "n");
  result.replace("Ñ", "N");
  result.replace("¿", "¿");
  result.replace("?", "?");
  result.replace("¡", "i"); // ¡ → i (sounds like 'i')

  // Other common
  result.replace("ü", "u");
  result.replace("Ü", "U");
  result.replace("ö", "o");
  result.replace("Ö", "O");
  result.replace("ä", "a");
  result.replace("Ä", "A");

  return result;
}

void KeyType::push(const QString &chunk) {
  QString newChunk = extractNewChunk(chunk);
  if (newChunk.isEmpty()) {
    return;
  }
  // Normalize to ASCII before typing
  QString normalized = normalizeText(newChunk);
  m_queue.enqueue(normalized);
  processQueue();
}

void KeyType::processQueue() {
  if (m_processing || m_queue.isEmpty()) {
    return;
  }

  m_processing = true;

  QString chunk = m_queue.dequeue();
  QString program = "./bin/ydotool/ydotool";
  QStringList args = {"type", chunk};

  m_process->setProgram(program);
  m_process->setArguments(args);

  QObject::connect(
      m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
      this, [this](int, QProcess::ExitStatus) {
        m_processing = false;
        processQueue();
      });

  m_process->start();
  m_process->waitForFinished(-1);
}
