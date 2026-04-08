#include "process_utils.hpp"
#include <stdio.h>

ProcessUtils::ProcessUtils(QObject *parent) : QObject(parent) {
}

ProcessUtils::~ProcessUtils() = default;

QProcess *ProcessUtils::createGladysProcess() {
  QProcess *proc = new QProcess();
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert("QT_QPA_PLATFORM", "xcb");
  proc->setProcessEnvironment(env);
  proc->setProgram("bin/gladys");
  return proc;
}

void ProcessUtils::launchGladys() {
  QProcess *proc = createGladysProcess();
  proc->start();
  fprintf(stderr, "ProcessUtils: gladys launched.\n");
}