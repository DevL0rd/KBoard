#pragma once

#include "modelcatalog.h"

#include <QFileSystemWatcher>
#include <QObject>
#include <QVariantList>

class ModelDownloader;

class VoiceModels : public QObject
{
    Q_OBJECT

public:
    explicit VoiceModels(QObject *parent = nullptr);

    QString catalogError() const;
    const ModelEntry *current() const;
    const ModelEntry *find(const QString &id) const;
    bool isReady(const ModelEntry &entry) const;
    QString pathFor(const ModelEntry &entry) const;
    QString vadPath() const;
    QVariantList list(const QString &loadedPath) const;

    double progress() const;
    QString downloadingId() const;
    QString downloadError() const;
    void download(const QString &id);
    void cancelDownload();
    bool remove(const QString &id);
    void setDownloadError(const QString &error);

Q_SIGNALS:
    void changed();
    void progressChanged();
    void downloadErrorChanged();
    void downloaded(const QString &id);

private:
    QString m_catalogError;
    ModelCatalog m_catalog;
    QString m_directory;
    QFileSystemWatcher m_watcher;
    ModelDownloader *m_downloader;
    QString m_downloadingId;
    QString m_downloadError;
};
