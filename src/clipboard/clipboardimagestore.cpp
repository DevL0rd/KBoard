#include "clipboardimagestore.h"
#include "clipboardhistoryfile.h"

#include <QCryptographicHash>
#include <QDebug>
#include <QDir>
#include <QSaveFile>
#include <QThreadPool>
#include <QtConcurrentRun>

namespace
{
constexpr int s_thumbnailSize = 480;

QThreadPool &ioPool()
{
    static QThreadPool pool;
    pool.setMaxThreadCount(1);
    return pool;
}

void writePrivateImage(const QImage &image, const QString &path)
{
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "kboard clipboard: cannot write" << path << file.errorString();
        return;
    }
    file.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner);
    if (!image.save(&file, "PNG") || !file.commit()) {
        qWarning() << "kboard clipboard: failed to save image" << path;
    }
}

QString idFromFileName(QString name)
{
    name.chop(4);
    if (name.endsWith(QLatin1String("-thumb"))) {
        name.chop(6);
    }
    return name;
}
}

ClipboardImageStore::ClipboardImageStore(QString directory)
    : m_directory(std::move(directory))
{
    ClipboardHistoryFile::ensurePrivateDirectory(m_directory);
}

ClipboardImageStore::~ClipboardImageStore()
{
    waitForWrites();
}

QByteArray ClipboardImageStore::fingerprint(const QImage &image)
{
    const QImage normalized = image.convertToFormat(QImage::Format_ARGB32);
    QCryptographicHash hash(QCryptographicHash::Sha256);
    hash.addData(QByteArray(QByteArray::number(normalized.width()) + 'x' + QByteArray::number(normalized.height())));
    hash.addData(QByteArrayView(normalized.constBits(), normalized.sizeInBytes()));
    return hash.result().toHex();
}

QString ClipboardImageStore::directory() const
{
    return m_directory;
}

QString ClipboardImageStore::pathFor(const QString &id, bool thumbnail) const
{
    return m_directory + QLatin1Char('/') + id + (thumbnail ? QStringLiteral("-thumb.png") : QStringLiteral(".png"));
}

void ClipboardImageStore::insert(const QString &id, const QImage &image)
{
    const bool large = image.width() > s_thumbnailSize || image.height() > s_thumbnailSize;
    Record record {large ? image.scaled(s_thumbnailSize, s_thumbnailSize, Qt::KeepAspectRatio, Qt::SmoothTransformation) : image, image};
    const QMutexLocker locker(&m_mutex);
    m_records.insert(id, record);
}

bool ClipboardImageStore::restore(const QString &id)
{
    const QImage thumbnail(pathFor(id, true));
    if (thumbnail.isNull()) {
        qWarning() << "kboard clipboard: dropping image entry without thumbnail" << id;
        return false;
    }
    const QMutexLocker locker(&m_mutex);
    m_records.insert(id, Record {thumbnail, QImage()});
    return true;
}

QImage ClipboardImageStore::image(const QString &id, bool thumbnail) const
{
    {
        const QMutexLocker locker(&m_mutex);
        const auto it = m_records.constFind(id);
        if (it == m_records.constEnd()) {
            return {};
        }
        const QImage &cached = thumbnail ? it->thumbnail : it->full;
        if (!cached.isNull()) {
            return cached;
        }
    }
    QImage loaded(pathFor(id, thumbnail));
    if (loaded.isNull()) {
        qWarning() << "kboard clipboard: image missing on disk" << pathFor(id, thumbnail);
    }
    return loaded;
}

void ClipboardImageStore::persist(const QString &id)
{
    Record record;
    {
        const QMutexLocker locker(&m_mutex);
        record = m_records.value(id);
    }
    const QString fullPath = pathFor(id, false);
    if (record.full.isNull() || QFile::exists(fullPath)) {
        return;
    }
    const QString thumbnailPath = pathFor(id, true);
    track(QtConcurrent::run(&ioPool(), [record, fullPath, thumbnailPath] {
        writePrivateImage(record.full, fullPath);
        writePrivateImage(record.thumbnail, thumbnailPath);
    }));
}

void ClipboardImageStore::drop(const QString &id)
{
    {
        const QMutexLocker locker(&m_mutex);
        m_records.remove(id);
    }
    const QString fullPath = pathFor(id, false);
    const QString thumbnailPath = pathFor(id, true);
    track(QtConcurrent::run(&ioPool(), [fullPath, thumbnailPath] {
        QFile::remove(fullPath);
        QFile::remove(thumbnailPath);
    }));
}

void ClipboardImageStore::prune(const QSet<QString> &keep)
{
    const QString directory = m_directory;
    track(QtConcurrent::run(&ioPool(), [directory, keep] {
        const auto files = QDir(directory).entryList({QStringLiteral("*.png")}, QDir::Files);
        for (const QString &name : files) {
            if (!keep.contains(idFromFileName(name))) {
                QFile::remove(directory + QLatin1Char('/') + name);
            }
        }
    }));
}

void ClipboardImageStore::track(const QFuture<void> &future)
{
    m_writes.removeIf([](const QFuture<void> &pending) { return pending.isFinished(); });
    m_writes.append(future);
}

void ClipboardImageStore::waitForWrites()
{
    for (QFuture<void> &future : m_writes) {
        future.waitForFinished();
    }
    m_writes.clear();
}
