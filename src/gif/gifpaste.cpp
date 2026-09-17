#include "gifpaste.h"

#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QSaveFile>
#include <QUrl>

#include <algorithm>

namespace GifPaste
{

QMimeData *buildMimeData(const QByteArray &gif, const QString &filePath)
{
    auto *mime = new QMimeData;
    const QByteArray fileUrl = QUrl::fromLocalFile(filePath).toEncoded();
    mime->setData(gifMimeType, gif);
    mime->setData(uriListMimeType, fileUrl + "\r\n");
    mime->setData(gnomeCopiedFilesMimeType, "copy\n" + fileUrl);
    return mime;
}

QString cacheFileName(const Klipy::Item &item)
{
    const QByteArray hash = QCryptographicHash::hash(item.shareGif.url.toUtf8(), QCryptographicHash::Sha1).toHex().left(12);
    QString slug = item.slug;
    slug.replace(QRegularExpression(QStringLiteral("[^A-Za-z0-9_-]")), QStringLiteral("-"));
    if (slug.isEmpty()) {
        slug = QStringLiteral("gif");
    }
    return slug.left(64) + QLatin1Char('-') + QString::fromLatin1(hash) + QStringLiteral(".gif");
}

bool isGif(const QByteArray &bytes)
{
    return bytes.startsWith("GIF87a") || bytes.startsWith("GIF89a");
}

bool writeCacheFile(const QString &path, const QByteArray &bytes, QString *error)
{
    QDir().mkpath(QFileInfo(path).absolutePath());
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly) || file.write(bytes) != bytes.size() || !file.commit()) {
        *error = QObject::tr("Could not save the GIF to %1: %2").arg(path, file.errorString());
        return false;
    }
    return true;
}

void touch(const QString &path)
{
    QFile file(path);
    if (file.open(QIODevice::ReadWrite)) {
        file.setFileTime(QDateTime::currentDateTimeUtc(), QFileDevice::FileModificationTime);
    }
}

qint64 trimCache(const QString &directory, qint64 maxBytes, const QString &keepPath)
{
    QFileInfoList files = QDir(directory).entryInfoList({QStringLiteral("*.gif")}, QDir::Files);
    std::sort(files.begin(), files.end(), [](const QFileInfo &a, const QFileInfo &b) { return a.lastModified() > b.lastModified(); });
    qint64 total = 0;
    const QString keep = QFileInfo(keepPath).absoluteFilePath();
    for (const QFileInfo &info : std::as_const(files)) {
        if (info.absoluteFilePath() == keep) {
            total += info.size();
        }
    }
    for (const QFileInfo &info : std::as_const(files)) {
        if (info.absoluteFilePath() == keep) {
            continue;
        }
        if (total + info.size() > maxBytes) {
            QFile::remove(info.absoluteFilePath());
        } else {
            total += info.size();
        }
    }
    return total;
}

}
