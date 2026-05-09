#include "keytype.h"

#include <QCoreApplication>
#include <QEventLoop>

KeyType *KeyType::s_instance = nullptr;

KeyType::KeyType(QObject *parent) : QObject(parent), m_processing(false), m_process(nullptr) {
}

void KeyType::reset() {
  m_lastTyped.clear();
  m_queue.clear();
  m_processing = false;
}

KeyType *KeyType::instance() {
  if (!s_instance) {
    s_instance = new KeyType();
  }
  return s_instance;
}

QString KeyType::extractNewChunk(const QString &newText) {
  if (m_lastTyped.isEmpty()) {
    QString trimmed = newText.trimmed();
    m_lastTyped = newText;
    return trimmed;
  }

  int lastLen = m_lastTyped.length();
  if (newText.length() <= lastLen) {
    return QString();
  }

  QString diff = newText.mid(lastLen);
  diff = diff.trimmed();
  if (diff.isEmpty()) {
    return QString();
  }

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
  result.replace("¿", "");
  result.replace("¡", "");
  result.replace("?", "_");

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
  QString normalized = normalizeText(newChunk);
  m_queue.enqueue(normalized);
  processQueue();
}

void KeyType::runTyping(const QString &chunk) {
  if (m_process) {
    delete m_process;
  }
  m_process = new QProcess();
  m_process->setProgram("./bin/ydotool/ydotool");
  m_process->setArguments({"type", chunk});

  fprintf(stderr, "KeyType: Running typing: '%s'\n", qPrintable(chunk));
  m_process->start();

  while (m_process->state() == QProcess::Running) {
    QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
  }

  fprintf(stderr, "KeyType: Process finished\n");
  m_processing = false;
  processQueue();
}

void KeyType::processQueue() {
  if (m_processing || m_queue.isEmpty()) {
    return;
  }

  m_processing = true;
  QString chunk = m_queue.dequeue();
  runTyping(chunk);
}