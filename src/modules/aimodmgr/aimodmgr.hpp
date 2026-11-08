// AI Models - Installs and uninstalls the LLM (GGUF) and ASR (sherpa-onnx) models in the user
// cache. Downloads are SHA-256 checked and staged as .part, so an interrupted install never shows
// as installed.

#ifndef AIMODMGR_HPP
#define AIMODMGR_HPP

#include <QCryptographicHash>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QString>
#include <QVariantList>
#include <map>
#include <memory>

#include "catalog.hpp"

QString models_dir(ModelKind kind);          // cache folder of the kind
QString default_model_path(ModelKind kind);  // installed path of the model Gladys runs with

// Settings bridge: kind is 0 for LLM and 1 for ASR, as ModelKind. Runs on the GUI thread.
class ModelsBridge : public QObject {
  Q_OBJECT

 public:
  explicit ModelsBridge(QObject* parent = nullptr);
  ~ModelsBridge() override;

  // Rows: file, name, installed, inUse, busy, progress (percent while busy).
  Q_INVOKABLE QVariantList models(int kind) const;
  Q_INVOKABLE void install(int kind, const QString& file);
  Q_INVOKABLE void uninstall(int kind, const QString& file);

 signals:
  void changed();  // an install started, or one finished or failed, or a model was removed
  void progressChanged(const QString& file, int percent);
  void failed(const QString& file, const QString& reason);

 private:
  struct Job {  // one install in flight
    ModelInfo model;
    QFile file;  // the download, written as bytes arrive
    QCryptographicHash hash{QCryptographicHash::Sha256};
    QNetworkReply* reply = nullptr;
    QString error;  // set when a local write fails
    int percent = 0;
  };

  Job* find_job(const QString& file) const;
  void on_data(const QString& file);
  void on_progress(const QString& file, qint64 received, qint64 total);
  void on_downloaded(const QString& file);
  void extract(const QString& file);
  void on_extracted(const QString& file, bool ok);
  void complete(const QString& file);
  void fail(const QString& file, const QString& reason);

  std::map<QString, std::unique_ptr<Job>>
    m_jobs;  // by file; declared before the network so it outlives it
  QNetworkAccessManager m_network;
};

#endif  // AIMODMGR_HPP
