#include "voicemodels.h"

#include "kboardsettings.h"
#include "modeldownloader.h"

#include <QDir>
#include <QFile>

VoiceModels::VoiceModels(QObject *parent)
    : QObject(parent)
    , m_catalog(ModelCatalog::fromFile(ModelCatalog::defaultCatalogPath(), &m_catalogError))
    , m_directory(ModelCatalog::defaultModelsDirectory())
    , m_downloader(new ModelDownloader(this))
{
    QDir().mkpath(m_directory);
    m_watcher.addPath(m_directory);
    connect(&m_watcher, &QFileSystemWatcher::directoryChanged, this, &VoiceModels::changed);
    connect(m_downloader, &ModelDownloader::progressChanged, this, &VoiceModels::progressChanged);
    connect(m_downloader, &ModelDownloader::entryFinished, this, &VoiceModels::changed);
    connect(m_downloader, &ModelDownloader::finished, this, [this]() {
        const QString id = m_downloadingId;
        m_downloadingId.clear();
        Q_EMIT progressChanged();
        Q_EMIT changed();
        Q_EMIT downloaded(id);
    });
    connect(m_downloader, &ModelDownloader::failed, this, [this](const QString &, const QString &error) {
        m_downloadingId.clear();
        setDownloadError(error);
        Q_EMIT progressChanged();
        Q_EMIT changed();
    });
    connect(m_downloader, &ModelDownloader::canceled, this, [this]() {
        m_downloadingId.clear();
        Q_EMIT progressChanged();
        Q_EMIT changed();
    });
}

QString VoiceModels::catalogError() const
{
    return m_catalogError;
}

QString VoiceModels::speedReference() const
{
    return m_catalog.speedReference();
}

const ModelEntry *VoiceModels::current() const
{
    return find(KBoardSettings::voiceModel());
}

const ModelEntry *VoiceModels::find(const QString &id) const
{
    const ModelEntry *entry = m_catalog.find(id);
    return entry && entry->engine != u"silero-vad" ? entry : nullptr;
}

bool VoiceModels::isReady(const ModelEntry &entry) const
{
    return ModelCatalog::isDownloaded(entry, m_directory) && ModelCatalog::isDownloaded(m_catalog.vad(), m_directory);
}

QString VoiceModels::pathFor(const ModelEntry &entry) const
{
    return ModelCatalog::pathFor(entry, m_directory);
}

QString VoiceModels::vadPath() const
{
    return ModelCatalog::pathFor(m_catalog.vad(), m_directory);
}

QVariantList VoiceModels::list(const QString &loadedPath) const
{
    QVariantList result;
    const QString current = KBoardSettings::voiceModel();
    for (const ModelEntry &entry : m_catalog.models()) {
        QVariantMap map = entry.toVariantMap();
        map.insert(QStringLiteral("downloaded"), ModelCatalog::isDownloaded(entry, m_directory));
        map.insert(QStringLiteral("active"), entry.id == current);
        map.insert(QStringLiteral("loaded"), !loadedPath.isEmpty() && loadedPath == pathFor(entry));
        result.append(map);
    }
    return result;
}

double VoiceModels::progress() const
{
    return m_downloader->isBusy() ? m_downloader->progress() : -1.0;
}

QString VoiceModels::downloadingId() const
{
    return m_downloadingId;
}

QString VoiceModels::downloadError() const
{
    return m_downloadError;
}

void VoiceModels::download(const QString &id)
{
    const ModelEntry *entry = find(id);
    if (!entry) {
        setDownloadError(
            m_catalogError.isEmpty() ? QStringLiteral("The voice model \"%1\" is not in the model list").arg(id) : m_catalogError);
        return;
    }
    if (m_downloader->isBusy()) {
        setDownloadError(QStringLiteral("Another model download is already running"));
        return;
    }
    setDownloadError(QString());
    QList<ModelEntry> queue;
    if (!ModelCatalog::isDownloaded(m_catalog.vad(), m_directory)) {
        queue.append(m_catalog.vad());
    }
    if (!ModelCatalog::isDownloaded(*entry, m_directory)) {
        queue.append(*entry);
    }
    if (queue.isEmpty()) {
        Q_EMIT changed();
        Q_EMIT downloaded(id);
        return;
    }
    m_downloadingId = id;
    m_downloader->fetch(queue, m_directory);
    Q_EMIT progressChanged();
}

void VoiceModels::cancelDownload()
{
    m_downloader->cancel();
}

bool VoiceModels::remove(const QString &id)
{
    const ModelEntry *entry = find(id);
    if (!entry || m_downloadingId == id) {
        return false;
    }
    const bool removed = QFile::remove(pathFor(*entry));
    Q_EMIT changed();
    return removed;
}

void VoiceModels::setDownloadError(const QString &error)
{
    if (m_downloadError == error) {
        return;
    }
    m_downloadError = error;
    Q_EMIT downloadErrorChanged();
}
