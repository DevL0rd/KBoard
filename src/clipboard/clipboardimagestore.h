#pragma once

#include <QFuture>
#include <QHash>
#include <QImage>
#include <QMutex>
#include <QSet>
#include <QString>

class ClipboardImageStore
{
public:
    explicit ClipboardImageStore(QString directory);
    ~ClipboardImageStore();

    ClipboardImageStore(const ClipboardImageStore &) = delete;
    ClipboardImageStore &operator=(const ClipboardImageStore &) = delete;

    static QByteArray fingerprint(const QImage &image);

    QString directory() const;
    void insert(const QString &id, const QImage &image);
    bool restore(const QString &id);
    QImage image(const QString &id, bool thumbnail) const;
    void persist(const QString &id);
    void drop(const QString &id);
    void prune(const QSet<QString> &keep);
    void waitForWrites();

private:
    struct Record
    {
        QImage thumbnail;
        QImage full;
    };

    QString pathFor(const QString &id, bool thumbnail) const;
    void track(const QFuture<void> &future);

    QString m_directory;
    QHash<QString, Record> m_records;
    mutable QMutex m_mutex;
    QList<QFuture<void>> m_writes;
};
