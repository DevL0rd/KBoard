#pragma once

#include "modelcatalog.h"

#include <QCryptographicHash>
#include <QFile>
#include <QObject>
#include <QPointer>

class QNetworkAccessManager;
class QNetworkReply;

class ModelDownloader : public QObject
{
    Q_OBJECT

public:
    explicit ModelDownloader(QObject *parent = nullptr);
    ~ModelDownloader() override;

    void fetch(const QList<ModelEntry> &entries, const QString &directory);
    void cancel();
    bool isBusy() const;
    QString currentId() const;
    double progress() const;

Q_SIGNALS:
    void progressChanged(double fraction);
    void entryFinished(const QString &id);
    void finished();
    void failed(const QString &id, const QString &error);
    void canceled();

private:
    void startNext();
    void onReadyRead();
    void onFinished();
    void abortWith(const QString &error);
    void cleanupReply();

    QNetworkAccessManager *m_network;
    QPointer<QNetworkReply> m_reply;
    QList<ModelEntry> m_queue;
    ModelEntry m_current;
    QString m_directory;
    QFile m_part;
    QCryptographicHash m_hash {QCryptographicHash::Sha256};
    qint64 m_totalBytes = 0;
    qint64 m_doneBytes = 0;
    qint64 m_currentBytes = 0;
    double m_progress = -1.0;
};
