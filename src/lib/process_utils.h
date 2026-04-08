#ifndef PROCESS_UTILS_HPP
#define PROCESS_UTILS_HPP

#include <QObject>
#include <QProcess>

class ProcessUtils : public QObject {
  Q_OBJECT

public:
  explicit ProcessUtils(QObject *parent = nullptr);
  ~ProcessUtils() override;

  void launchGladys();

private:
  QProcess *createGladysProcess();
};

#endif // PROCESS_UTILS_HPP