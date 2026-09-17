#pragma once

#include "klipyapi.h"

#include <QObject>

class KlipyClient;

class GifInserter : public QObject
{
    Q_OBJECT

public:
    enum class Mode
    {
        Image,
        Link,
    };

    GifInserter(KlipyClient *client, const QString &cacheDirectory, QObject *parent = nullptr);

    QString insertingId() const;
    void setCacheLimit(qint64 bytes);
    void insert(const Klipy::Item &item, Mode mode);

Q_SIGNALS:
    void insertingChanged();
    void inserted(const Klipy::Item &item);
    void failed(const QString &id, const QString &error);

private:
    void download(const Klipy::Item &item, const QString &path);
    void store(const Klipy::Item &item, const QString &path, const QByteArray &bytes);
    void paste(const Klipy::Item &item, const QString &path, const QByteArray &bytes);
    void fail(const QString &id, const QString &error);
    void setInsertingId(const QString &id);

    KlipyClient *m_client;
    QString m_cacheDirectory;
    qint64 m_cacheLimit = qint64(64) * 1024 * 1024;
    QString m_insertingId;
};
