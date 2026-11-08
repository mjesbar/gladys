// AI Models - Installs and uninstalls the LLM (GGUF) and ASR (sherpa-onnx) models in the user
// cache.

#include "aimodmgr.hpp"

#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QUrl>
#include <algorithm>

#include "../log/log.hpp"

QString models_dir(ModelKind kind) {
  const bool llm = kind == ModelKind::Llm;
#if defined(Q_OS_WIN)
  return qEnvironmentVariable("LOCALAPPDATA") +
    (llm ? "/Gladys/Cache/Models/LLM" : "/Gladys/Cache/Models/ASR");
#elif defined(Q_OS_MACOS)
  return QDir::homePath() +
    (llm ? "/Library/Caches/Gladys/Models/LLM" : "/Library/Caches/Gladys/Models/ASR");
#else
  return QDir::homePath() + (llm ? "/.cache/gladys/models/llm" : "/.cache/gladys/models/asr");
#endif
}

static const ModelInfo* find_model(int kind, const QString& file) {
  for (const ModelInfo& model : MODEL_CATALOG) {
    if (static_cast<int>(model.kind) == kind && model.file == file) {
      return &model;
    }
  }
  return nullptr;
}

static QString installed_path(const ModelInfo& model) {
  return models_dir(model.kind) + "/" + model.file;
}

// GGUF downloads straight to <file>.part; sherpa-onnx downloads the archive, then extracts to
// <file>.extract.
static QString download_path(const ModelInfo& model) {
  return installed_path(model) + (model.kind == ModelKind::Asr ? ".tar.bz2.part" : ".part");
}

static QString extract_path(const ModelInfo& model) { return installed_path(model) + ".extract"; }

static bool is_installed(const ModelInfo& model) {
  return QFileInfo::exists(installed_path(model));
}

static int installed_count(ModelKind kind) {
  return std::count_if(MODEL_CATALOG.begin(), MODEL_CATALOG.end(), [kind](const ModelInfo& model) {
    return model.kind == kind && is_installed(model);
  });
}

// Removes what a failed install left behind.
static void discard(const ModelInfo& model) {
  QFile::remove(download_path(model));
  if (model.kind == ModelKind::Asr) {
    QDir(extract_path(model)).removeRecursively();
  }
}

QString default_model_path(ModelKind kind) {
  for (const ModelInfo& model : MODEL_CATALOG) {
    if (model.kind == kind && model.is_default) {
      return installed_path(model);
    }
  }
  return {};
}

ModelsBridge::ModelsBridge(QObject* parent) : QObject(parent) {}

// Jobs go first: the network manager then aborts their replies, which find no job and do nothing.
ModelsBridge::~ModelsBridge() { m_jobs.clear(); }

QVariantList ModelsBridge::models(int kind) const {
  QVariantList rows;
  for (const ModelInfo& model : MODEL_CATALOG) {
    if (static_cast<int>(model.kind) != kind) {
      continue;
    }
    const Job* job = find_job(model.file);
    rows.append(QVariantMap{{"file", model.file},
      {"name", model.name},
      {"installed", is_installed(model)},
      {"inUse", model.is_default},
      {"busy", job != nullptr},
      {"progress", job ? job->percent : 0}});
  }
  return rows;
}

void ModelsBridge::install(int kind, const QString& file) {
  const ModelInfo* model = find_model(kind, file);
  if (!model || m_jobs.count(file) || is_installed(*model)) {
    return;
  }
  QDir().mkpath(models_dir(model->kind));
  auto job = std::make_unique<Job>();
  job->model = *model;
  job->file.setFileName(download_path(*model));
  if (!job->file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    emit failed(file, "cannot write " + job->file.fileName());
    return;
  }
  job->reply = m_network.get(QNetworkRequest(QUrl(model->url)));
  connect(job->reply, &QNetworkReply::readyRead, this, [this, file]() { on_data(file); });
  connect(job->reply,
    &QNetworkReply::downloadProgress,
    this,
    [this, file](qint64 received, qint64 total) { on_progress(file, received, total); });
  connect(job->reply, &QNetworkReply::finished, this, [this, file]() { on_downloaded(file); });
  m_jobs.emplace(file, std::move(job));
  emit changed();
}

void ModelsBridge::uninstall(int kind, const QString& file) {
  const ModelInfo* model = find_model(kind, file);
  if (!model || m_jobs.count(file) || !is_installed(*model)) {
    return;
  }
  if (model->is_default) {
    emit failed(file, tr("In use by Gladys"));
    return;
  }
  if (installed_count(model->kind) <= 1) {
    emit failed(file, tr("Keep at least one model installed"));
    return;
  }
  const QString path = installed_path(*model);
  const bool removed =
    QFileInfo(path).isDir() ? QDir(path).removeRecursively() : QFile::remove(path);
  if (!removed) {
    emit failed(file, tr("Cannot remove %1").arg(path));
    return;
  }
  qCInfo(lcMain) << "model removed:" << file;
  emit changed();
}

ModelsBridge::Job* ModelsBridge::find_job(const QString& file) const {
  const auto it = m_jobs.find(file);
  return it == m_jobs.end() ? nullptr : it->second.get();
}

void ModelsBridge::on_data(const QString& file) {
  Job* job = find_job(file);
  if (!job) {
    return;
  }
  const QByteArray chunk = job->reply->readAll();
  job->hash.addData(chunk);
  if (job->file.write(chunk) != chunk.size()) {
    job->error = "cannot write " + job->file.fileName();
    job->reply->abort();  // finished follows and reports the error
  }
}

// Emits only when the whole percent changes, so the UI is not flooded with chunk-sized updates.
void ModelsBridge::on_progress(const QString& file, qint64 received, qint64 total) {
  Job* job = find_job(file);
  if (!job || total <= 0) {
    return;
  }
  const int percent = static_cast<int>(received * 100 / total);
  if (percent != job->percent) {
    job->percent = percent;
    emit progressChanged(file, percent);
  }
}

// Checks the SHA-256, then places the file: GGUF is renamed into place, a sherpa-onnx archive is
// extracted.
void ModelsBridge::on_downloaded(const QString& file) {
  Job* job = find_job(file);
  if (!job) {
    return;
  }
  QNetworkReply* reply = job->reply;
  job->reply = nullptr;
  reply->deleteLater();
  job->file.close();

  QString error = job->error;
  if (error.isEmpty() && reply->error() != QNetworkReply::NoError) {
    error = reply->errorString();
  }
  if (error.isEmpty() && job->hash.result().toHex() != job->model.sha256.toLatin1()) {
    error = "checksum mismatch";
  }
  if (!error.isEmpty()) {
    fail(file, error);
    return;
  }
  if (job->model.kind == ModelKind::Asr) {
    extract(file);
    return;
  }
  if (!QFile::rename(download_path(job->model), installed_path(job->model))) {
    fail(file, "cannot place the model");
    return;
  }
  complete(file);
}

// Extracts the archive into a staging folder, dropping its single top folder, so the final name
// appears only when complete.
void ModelsBridge::extract(const QString& file) {
  const ModelInfo model = find_job(file)->model;
  QDir().mkpath(extract_path(model));
  auto* tar = new QProcess(this);
  connect(tar, &QProcess::finished, this, [this, file, tar](int code, QProcess::ExitStatus status) {
    tar->deleteLater();
    on_extracted(file, code == 0 && status == QProcess::NormalExit);
  });
  connect(tar, &QProcess::errorOccurred, this, [this, file, tar]() {
    tar->deleteLater();
    on_extracted(file, false);
  });
  tar->start(
    "tar", {"-xjf", download_path(model), "-C", extract_path(model), "--strip-components=1"});
}

void ModelsBridge::on_extracted(const QString& file, bool ok) {
  Job* job = find_job(file);
  if (!job) {
    return;
  }
  const ModelInfo model = job->model;
  QFile::remove(download_path(model));
  if (!ok || !QDir().rename(extract_path(model), installed_path(model))) {
    fail(file, "extraction failed");
    return;
  }
  complete(file);
}

void ModelsBridge::complete(const QString& file) {
  qCInfo(lcMain) << "model installed:" << file;
  m_jobs.erase(file);
  emit changed();
}

void ModelsBridge::fail(const QString& file, const QString& reason) {
  Job* job = find_job(file);
  if (!job) {
    return;
  }
  discard(job->model);
  qCWarning(lcMain) << "model install failed:" << file << reason;
  m_jobs.erase(file);
  emit changed();
  emit failed(file, reason);
}
