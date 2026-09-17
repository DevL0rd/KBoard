#include "gifinserter.h"
#include "gifpaste.h"
#include "klipyclient.h"

#include "inputcontext.h"

#include <KSystemClipboard>

#include <QFile>

GifInserter::GifInserter(KlipyClient *client, const QString &cacheDirectory, QObject *parent)
    : QObject(parent)
    , m_client(client)
    , m_cacheDirectory(cacheDirectory)
{ }

QString GifInserter::insertingId() const
{
    return m_insertingId;
}

void GifInserter::setCacheLimit(qint64 bytes)
{
    m_cacheLimit = bytes;
}

void GifInserter::insert(const Klipy::Item &item, Mode mode)
{
    if (!m_insertingId.isEmpty()) {
        return;
    }
    if (!InputContext::instance()->isActive()) {
        fail(item.id, tr("No text field is focused"));
        return;
    }
    if (mode == Mode::Link) {
        InputContext::instance()->commit(item.linkGif.url);
        Q_EMIT inserted(item);
        return;
    }
    const QString path = m_cacheDirectory + QLatin1Char('/') + GifPaste::cacheFileName(item);
    QFile cached(path);
    if (cached.open(QIODevice::ReadOnly)) {
        const QByteArray bytes = cached.readAll();
        cached.close();
        if (GifPaste::isGif(bytes)) {
            GifPaste::touch(path);
            paste(item, path, bytes);
            return;
        }
    }
    download(item, path);
}

void GifInserter::download(const Klipy::Item &item, const QString &path)
{
    setInsertingId(item.id);
    auto onSuccess = [this, item, path](const QByteArray &bytes) { store(item, path, bytes); };
    auto onError = [this, id = item.id](const QString &error) { fail(id, tr("Could not download the GIF: %1").arg(error)); };
    if (!m_client->get(QUrl(item.shareGif.url), onSuccess, onError)) {
        fail(item.id, tr("GIF downloads are disabled while previewing fixtures"));
    }
}

void GifInserter::store(const Klipy::Item &item, const QString &path, const QByteArray &bytes)
{
    if (!GifPaste::isGif(bytes)) {
        fail(item.id, tr("KLIPY did not return a GIF file"));
        return;
    }
    QString error;
    if (!GifPaste::writeCacheFile(path, bytes, &error)) {
        fail(item.id, error);
        return;
    }
    GifPaste::trimCache(m_cacheDirectory, m_cacheLimit, path);
    paste(item, path, bytes);
}

void GifInserter::paste(const Klipy::Item &item, const QString &path, const QByteArray &bytes)
{
    KSystemClipboard *clipboard = KSystemClipboard::instance();
    if (!clipboard) {
        fail(item.id, tr("The system clipboard is not available"));
        return;
    }
    clipboard->setMimeData(GifPaste::buildMimeData(bytes, path), QClipboard::Clipboard);
    if (!InputContext::instance()->shortcut(QStringLiteral("ctrl+v"))) {
        fail(item.id, tr("Could not send Ctrl+V to the focused app"));
        return;
    }
    setInsertingId({});
    Q_EMIT inserted(item);
}

void GifInserter::fail(const QString &id, const QString &error)
{
    setInsertingId({});
    Q_EMIT failed(id, error);
}

void GifInserter::setInsertingId(const QString &id)
{
    if (m_insertingId != id) {
        m_insertingId = id;
        Q_EMIT insertingChanged();
    }
}
