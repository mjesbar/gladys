// AI Models - Installs, uninstalls and selects the LLM (GGUF) and ASR (sherpa-onnx) models in the user cache.

#include "aimodmgr.hpp"

#include <QDir>
#include <QFileInfo>
#include <QMetaObject>
#include <QProcess>
#include <QUrl>
#include <algorithm>

#include "../data/data.hpp"
#include "../log/log.hpp"

QString models_dir(ModelKind kind) {
  const bool llm = kind == ModelKind::Llm;
#if defined(Q_OS_WIN)
  return qEnvironmentVariable("LOCALAPPDATA") + (llm ? "/Gladys/Cache/Models/LLM" : "/Gladys/Cache/Models/ASR");
#elif defined(Q_OS_MACOS)
  return QDir::homePath() + (llm ? "/Library/Caches/Gladys/Models/LLM" : "/Library/Caches/Gladys/Models/ASR");
#else
  return QDir::homePath() + (llm ? "/.cache/gladys/models/llm" : "/.cache/gladys/models/asr");
#endif
}

static const std::map<QString, Model>& catalog(ModelKind kind) {
  return kind == ModelKind::Llm ? LLM_CATALOG : ASR_CATALOG;
}

static const Model* find_model(ModelKind kind, const QString& key) {
  const auto& models = catalog(kind);
  const auto it = models.find(key);
  return it == models.end() ? nullptr : &it->second;
}

static QString installed_path(ModelKind kind, const QString& key) { return models_dir(kind) + "/" + key; }

// GGUF downloads straight to <key>.part; an ASR archive downloads to <key>.tar.bz2.part, then extracts to <key>.extract.
static QString download_path(ModelKind kind, const QString& key) {
  return installed_path(kind, key) + (kind == ModelKind::Asr ? ".tar.bz2.part" : ".part");
}

static QString extract_path(ModelKind kind, const QString& key) { return installed_path(kind, key) + ".extract"; }

static bool is_installed(ModelKind kind, const QString& key) { return QFileInfo::exists(installed_path(kind, key)); }

static int installed_count(ModelKind kind) {
  const auto& models = catalog(kind);
  return std::count_if(models.begin(), models.end(),
    [kind](const auto& entry) { return is_installed(kind, entry.first); });
}

// Removes what a failed install left behind.
static void discard(ModelKind kind, const QString& key) {
  QFile::remove(download_path(kind, key));
  if (kind == ModelKind::Asr) {
    QDir(extract_path(kind, key)).removeRecursively();
  }
}

// Selection is stored per kind in the settings table, as the catalog key of the model.
static QString selection_key(ModelKind kind) { return kind == ModelKind::Llm ? "llmModel" : "asrModel"; }

static QString stored_selection(ModelKind kind) {
  QVariantMap settings;
  QMetaObject::invokeMethod(Data::instance(), "read_settings", Qt::BlockingQueuedConnection,
    Q_RETURN_ARG(QVariantMap, settings));
  return settings.value(selection_key(kind)).toString();
}

static void store_selection(ModelKind kind, const QString& key) {
  bool ok = false;
  QMetaObject::invokeMethod(Data::instance(), "update_setting", Qt::BlockingQueuedConnection,
    Q_RETURN_ARG(bool, ok), Q_ARG(QString, selection_key(kind)), Q_ARG(QVariant, QVariant(key)));
  if (!ok) {
    QMetaObject::invokeMethod(Data::instance(), "create_setting", Qt::BlockingQueuedConnection,
      Q_RETURN_ARG(bool, ok), Q_ARG(QString, selection_key(kind)), Q_ARG(QVariant, QVariant(key)));
  }
}

// Key of the model Gladys runs: the stored selection when installed, else the default of the kind.
static QString selected_key(ModelKind kind) {
  const QString stored = stored_selection(kind);
  if (find_model(kind, stored) && is_installed(kind, stored)) {
    return stored;
  }
  for (const auto& [key, model] : catalog(kind)) {
    if (model.is_default) {
      return key;
    }
  }
  return {};
}

QString selected_model_path(ModelKind kind) {
  const QString key = selected_key(kind);
  return key.isEmpty() ? QString() : installed_path(kind, key);
}

ModelsBridge::ModelsBridge(QObject* parent) : QObject(parent) {}

// Jobs go first: the network manager then aborts their replies, which find no job and do nothing.
ModelsBridge::~ModelsBridge() { m_jobs.clear(); }

bool ModelsBridge::loading() const { return m_loading; }

void ModelsBridge::set_loading(bool loading) {
  if (m_loading == loading) {
    return;
  }
  m_loading = loading;
  emit loadingChanged();
}

QVariantList ModelsBridge::models(int kind) const {
  const ModelKind k = static_cast<ModelKind>(kind);
  const QString selected = selected_key(k);
  QVariantList rows;
  for (const auto& [key, model] : catalog(k)) {
    const Job* job = find_job(key);
    rows.append(QVariantMap{{"file", key},
      {"name", model.name},
      {"installed", is_installed(k, key)},
      {"selected", key == selected},
      {"busy", job != nullptr},
      {"progress", job ? job->percent : 0}});
  }
  return rows;
}

void ModelsBridge::install(int kind, const QString& file) {
  const ModelKind k = static_cast<ModelKind>(kind);
  const Model* model = find_model(k, file);
  if (!model || m_jobs.count(file) || is_installed(k, file)) {
    return;
  }
  QDir().mkpath(models_dir(k));
  auto job = std::make_unique<Job>();
  job->kind = k;
  job->key = file;
  job->model = *model;
  job->part.setFileName(download_path(k, file));
  if (!job->part.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    emit failed(file, "cannot write " + job->part.fileName());
    return;
  }
  job->reply = m_network.get(QNetworkRequest(QUrl(model->url)));
  connect(job->reply, &QNetworkReply::readyRead, this, [this, file]() { on_data(file); });
  connect(job->reply, &QNetworkReply::downloadProgress, this,
    [this, file](qint64 received, qint64 total) { on_progress(file, received, total); });
  connect(job->reply, &QNetworkReply::finished, this, [this, file]() { on_downloaded(file); });
  m_jobs.emplace(file, std::move(job));
  emit changed();
}

void ModelsBridge::uninstall(int kind, const QString& file) {
  const ModelKind k = static_cast<ModelKind>(kind);
  if (!find_model(k, file) || m_jobs.count(file) || !is_installed(k, file)) {
    return;
  }
  if (file == selected_key(k)) {
    emit failed(file, tr("Selected model cannot be uninstalled"));
    return;
  }
  if (installed_count(k) <= 1) {
    emit failed(file, tr("Keep at least one model installed"));
    return;
  }
  const QString path = installed_path(k, file);
  const bool removed = QFileInfo(path).isDir() ? QDir(path).removeRecursively() : QFile::remove(path);
  if (!removed) {
    emit failed(file, tr("Cannot remove %1").arg(path));
    return;
  }
  qCInfo(lcMain) << "model removed:" << file;
  emit changed();
}

// Refused while a model loads, so loads never overlap. Takes effect when the child restarts.
void ModelsBridge::select(int kind, const QString& file) {
  const ModelKind k = static_cast<ModelKind>(kind);
  if (!find_model(k, file) || m_loading || m_jobs.count(file) || !is_installed(k, file) || file == selected_key(k)) {
    return;
  }
  store_selection(k, file);
  qCInfo(lcMain) << "model selected:" << file;
  emit selected(kind);
  emit changed();
}

ModelsBridge::Job* ModelsBridge::find_job(const QString& key) const {
  const auto it = m_jobs.find(key);
  return it == m_jobs.end() ? nullptr : it->second.get();
}

void ModelsBridge::on_data(const QString& key) {
  Job* job = find_job(key);
  if (!job) {
    return;
  }
  const QByteArray chunk = job->reply->readAll();
  job->hash.addData(chunk);
  if (job->part.write(chunk) != chunk.size()) {
    job->error = "cannot write " + job->part.fileName();
    job->reply->abort();  // finished follows and reports the error
  }
}

// Emits only when the whole percent changes, so the UI is not flooded with chunk-sized updates.
void ModelsBridge::on_progress(const QString& key, qint64 received, qint64 total) {
  Job* job = find_job(key);
  if (!job || total <= 0) {
    return;
  }
  const int percent = static_cast<int>(received * 100 / total);
  if (percent != job->percent) {
    job->percent = percent;
    emit progressChanged(key, percent);
  }
}

// Checks the SHA-256, then places the file: GGUF is renamed into place, an ASR archive is extracted.
void ModelsBridge::on_downloaded(const QString& key) {
  Job* job = find_job(key);
  if (!job) {
    return;
  }
  QNetworkReply* reply = job->reply;
  job->reply = nullptr;
  reply->deleteLater();
  job->part.close();

  QString error = job->error;
  if (error.isEmpty() && reply->error() != QNetworkReply::NoError) {
    error = reply->errorString();
  }
  if (error.isEmpty() && job->hash.result().toHex() != job->model.sha256.toLatin1()) {
    error = "checksum mismatch";
  }
  if (!error.isEmpty()) {
    fail(key, error);
    return;
  }
  if (job->kind == ModelKind::Asr) {
    extract(key);
    return;
  }
  if (!QFile::rename(download_path(job->kind, key), installed_path(job->kind, key))) {
    fail(key, "cannot place the model");
    return;
  }
  complete(key);
}

// Extracts the archive into a staging folder. Its top folder is the catalog key, so it is renamed into place.
void ModelsBridge::extract(const QString& key) {
  const Job* job = find_job(key);
  QDir().mkpath(extract_path(job->kind, key));
  auto* tar = new QProcess(this);
  connect(tar, &QProcess::finished, this, [this, key, tar](int code, QProcess::ExitStatus status) {
    tar->deleteLater();
    on_extracted(key, code == 0 && status == QProcess::NormalExit);
  });
  connect(tar, &QProcess::errorOccurred, this, [this, key, tar]() {
    tar->deleteLater();
    on_extracted(key, false);
  });
  tar->start("tar", {"-xjf", download_path(job->kind, key), "-C", extract_path(job->kind, key)});
}

void ModelsBridge::on_extracted(const QString& key, bool ok) {
  Job* job = find_job(key);
  if (!job) {
    return;
  }
  const QString staged = extract_path(job->kind, key) + "/" + key;
  if (!ok || !QDir().rename(staged, installed_path(job->kind, key))) {
    fail(key, "extraction failed");
    return;
  }
  QDir(extract_path(job->kind, key)).removeRecursively();
  QFile::remove(download_path(job->kind, key));
  complete(key);
}

void ModelsBridge::complete(const QString& key) {
  qCInfo(lcMain) << "model installed:" << key;
  m_jobs.erase(key);
  emit changed();
}

void ModelsBridge::fail(const QString& key, const QString& reason) {
  Job* job = find_job(key);
  if (!job) {
    return;
  }
  discard(job->kind, key);
  qCWarning(lcMain) << "model install failed:" << key << reason;
  m_jobs.erase(key);
  emit changed();
  emit failed(key, reason);
}
