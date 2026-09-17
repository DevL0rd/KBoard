#pragma once

#include "klipyapi.h"

#include <QByteArray>
#include <QMimeData>
#include <QString>

namespace GifPaste
{

inline const QString gifMimeType = QStringLiteral("image/gif");
inline const QString uriListMimeType = QStringLiteral("text/uri-list");
inline const QString gnomeCopiedFilesMimeType = QStringLiteral("x-special/gnome-copied-files");

QMimeData *buildMimeData(const QByteArray &gif, const QString &filePath);
QString cacheFileName(const Klipy::Item &item);
bool isGif(const QByteArray &bytes);
bool writeCacheFile(const QString &path, const QByteArray &bytes, QString *error);
void touch(const QString &path);
qint64 trimCache(const QString &directory, qint64 maxBytes, const QString &keepPath);

}
