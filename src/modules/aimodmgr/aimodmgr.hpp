// AI Models - Installs, uninstalls and selects the LLM (GGUF) and ASR (sherpa-onnx) models in the user cache.
// Downloads are SHA-256 checked and staged as .part, so an interrupted install never shows as installed.

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

QString models_dir(ModelKind kind);           // cache folder of the kind
QString selected_model_path(ModelKind kind);  // installed path of the model Gladys runs with
bool needs_setup();                           // the default model of either kind is not installed

// Settings bridge: kind is 0 for LLM and 1 for ASR, as ModelKind. Runs on the GUI thread.
class ModelsBridge : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)

 public:
  explicit ModelsBridge(QObject* parent = nullptr);
  ~ModelsBridge() override;

  bool loading() const;

  // Rows: file, name, installed, selected, default, busy, progress (percent while busy).
  Q_INVOKABLE QVariantList models(int kind) const;
  Q_INVOKABLE void install(int kind, const QString& file);
  Q_INVOKABLE void uninstall(int kind, const QString& file);
  Q_INVOKABLE void select(int kind, const QString& file);

 public slots:
  void set_loading(bool loading);  // a model is loading: selection is refused

 signals:
  void changed();  // an install started, or one finished or failed, or a model was removed or selected
  void progressChanged(const QString& file, int percent);
  void failed(const QString& file, const QString& reason);
  void selected(int kind);  // the child of this kind must restart on its new model
  void loadingChanged();

 private:
  struct Job {  // one install in flight
    ModelKind kind;
    QString key;  // catalog key: file or folder name
    Model model;
    QFile part;  // the download, written as bytes arrive
    QCryptographicHash hash{QCryptographicHash::Sha256};
    QNetworkReply* reply = nullptr;
    QString error;  // set when a local write fails
    int percent = 0;
  };

  Job* find_job(const QString& key) const;
  void on_data(const QString& key);
  void on_progress(const QString& key, qint64 received, qint64 total);
  void on_downloaded(const QString& key);
  void extract(const QString& key);
  void on_extracted(const QString& key, bool ok);
  void complete(const QString& key);
  void fail(const QString& key, const QString& reason);

  std::map<QString, std::unique_ptr<Job>> m_jobs;  // by key; declared before the network so it outlives it
  QNetworkAccessManager m_network;
  bool m_loading = false;
};

#endif  // AIMODMGR_HPP
