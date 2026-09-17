#include "modeldownloader.h"

#include <QDir>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

ModelDownloader::ModelDownloader(QObject *parent)
    : QObject(parent)
    , m_network(new QNetworkAccessManager(this))
{ }

ModelDownloader::~ModelDownloader()
{
    if (m_reply) {
        cancel();
    }
}

void ModelDownloader::fetch(const QList<ModelEntry> &entries, const QString &directory)
{
    if (isBusy()) {
        Q_EMIT failed(entries.isEmpty() ? QString() : entries.first().id, QStringLiteral("Another model download is already running"));
        return;
    }
    m_queue = entries;
    m_directory = directory;
    m_totalBytes = 0;
    m_doneBytes = 0;
    for (const ModelEntry &entry : entries) {
        m_totalBytes += entry.sizeBytes;
    }
    if (!QDir().mkpath(directory)) {
        m_queue.clear();
        Q_EMIT failed(
            entries.isEmpty() ? QString() : entries.first().id, QStringLiteral("Cannot create the models folder %1").arg(directory));
        return;
    }
    startNext();
}

void ModelDownloader::cancel()
{
    if (!m_reply) {
        return;
    }
    QNetworkReply *reply = m_reply;
    cleanupReply();
    reply->abort();
    reply->deleteLater();
    m_part.remove();
    m_queue.clear();
    m_current = {};
    m_progress = -1.0;
    Q_EMIT progressChanged(m_progress);
    Q_EMIT canceled();
}

bool ModelDownloader::isBusy() const
{
    return !m_reply.isNull();
}

QString ModelDownloader::currentId() const
{
    return m_current.id;
}

double ModelDownloader::progress() const
{
    return m_progress;
}

void ModelDownloader::startNext()
{
    if (m_queue.isEmpty()) {
        m_current = {};
        m_progress = -1.0;
        Q_EMIT progressChanged(m_progress);
        Q_EMIT finished();
        return;
    }
    m_current = m_queue.takeFirst();
    m_currentBytes = 0;
    m_hash.reset();
    m_part.setFileName(ModelCatalog::pathFor(m_current, m_directory) + QStringLiteral(".part"));
    if (!m_part.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        const QString id = m_current.id;
        const QString error = QStringLiteral("Cannot write %1: %2").arg(m_part.fileName(), m_part.errorString());
        m_queue.clear();
        m_current = {};
        m_progress = -1.0;
        Q_EMIT progressChanged(m_progress);
        Q_EMIT failed(id, error);
        return;
    }
    QNetworkRequest request {QUrl(m_current.url)};
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("KBoard"));
    m_reply = m_network->get(request);
    connect(m_reply, &QNetworkReply::readyRead, this, &ModelDownloader::onReadyRead);
    connect(m_reply, &QNetworkReply::finished, this, &ModelDownloader::onFinished);
    m_progress = m_totalBytes > 0 ? double(m_doneBytes) / double(m_totalBytes) : 0.0;
    Q_EMIT progressChanged(m_progress);
}

void ModelDownloader::onReadyRead()
{
    if (!m_reply) {
        return;
    }
    const QByteArray chunk = m_reply->readAll();
    if (chunk.isEmpty()) {
        return;
    }
    if (m_part.write(chunk) != chunk.size()) {
        abortWith(QStringLiteral("Cannot write %1: %2").arg(m_part.fileName(), m_part.errorString()));
        return;
    }
    m_hash.addData(chunk);
    m_currentBytes += chunk.size();
    const double fraction = m_totalBytes > 0 ? qBound(0.0, double(m_doneBytes + m_currentBytes) / double(m_totalBytes), 1.0) : 0.0;
    if (fraction - m_progress >= 0.001 || fraction >= 1.0) {
        m_progress = fraction;
        Q_EMIT progressChanged(m_progress);
    }
}

void ModelDownloader::onFinished()
{
    if (!m_reply) {
        return;
    }
    onReadyRead();
    if (!m_reply) {
        return;
    }
    const QNetworkReply::NetworkError networkError = m_reply->error();
    const QString networkErrorString = m_reply->errorString();
    const int status = m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (networkError != QNetworkReply::NoError) {
        abortWith(QStringLiteral("Download of %1 failed: %2").arg(m_current.name, networkErrorString));
        return;
    }
    if (status != 200) {
        abortWith(QStringLiteral("Download of %1 failed: server answered HTTP %2").arg(m_current.name).arg(status));
        return;
    }
    if (m_currentBytes != m_current.sizeBytes) {
        abortWith(QStringLiteral("Download of %1 is incomplete: got %2 of %3 bytes")
                .arg(m_current.name)
                .arg(m_currentBytes)
                .arg(m_current.sizeBytes));
        return;
    }
    if (!m_current.sha256.isEmpty() && QString::fromLatin1(m_hash.result().toHex()) != m_current.sha256) {
        abortWith(QStringLiteral("Download of %1 is corrupted (checksum mismatch)").arg(m_current.name));
        return;
    }
    m_part.close();
    const QString finalPath = ModelCatalog::pathFor(m_current, m_directory);
    QFile::remove(finalPath);
    if (!m_part.rename(finalPath)) {
        abortWith(QStringLiteral("Cannot move %1 into place: %2").arg(m_part.fileName(), m_part.errorString()));
        return;
    }
    QNetworkReply *reply = m_reply;
    cleanupReply();
    reply->deleteLater();
    m_doneBytes += m_current.sizeBytes;
    const QString id = m_current.id;
    Q_EMIT entryFinished(id);
    startNext();
}

void ModelDownloader::abortWith(const QString &error)
{
    const QString id = m_current.id;
    if (m_reply) {
        QNetworkReply *reply = m_reply;
        cleanupReply();
        reply->abort();
        reply->deleteLater();
    }
    m_part.remove();
    m_queue.clear();
    m_current = {};
    m_progress = -1.0;
    Q_EMIT progressChanged(m_progress);
    Q_EMIT failed(id, error);
}

void ModelDownloader::cleanupReply()
{
    if (m_reply) {
        disconnect(m_reply, nullptr, this, nullptr);
    }
    m_reply = nullptr;
    if (m_part.isOpen()) {
        m_part.close();
    }
}
