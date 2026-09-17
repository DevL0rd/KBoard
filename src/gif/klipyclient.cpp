#include "klipyclient.h"

#include <QDir>
#include <QFile>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSaveFile>
#include <QUuid>

namespace
{

const int transferTimeoutMs = 20000;

QString storedCustomerId(const QString &path)
{
    QFile file(path);
    return file.open(QIODevice::ReadOnly) ? QString::fromUtf8(file.readAll()).trimmed() : QString();
}

}

KlipyClient::KlipyClient(Settings settings, QObject *parent)
    : QObject(parent)
    , m_settings(std::move(settings))
    , m_network(new QNetworkAccessManager(this))
{
    m_network->setAutoDeleteReplies(true);
    m_network->setTransferTimeout(transferTimeoutMs);
}

bool KlipyClient::isConfigured() const
{
    return !m_settings.apiKey.isEmpty();
}

bool KlipyClient::isOffline() const
{
    return m_offline;
}

void KlipyClient::setOffline(bool offline)
{
    m_offline = offline;
}

void KlipyClient::setContentFilter(Klipy::ContentFilter filter)
{
    m_settings.contentFilter = filter;
}

Klipy::ContentFilter KlipyClient::contentFilter() const
{
    return m_settings.contentFilter;
}

QString KlipyClient::customerId()
{
    if (!m_customerId.isEmpty()) {
        return m_customerId;
    }
    const QString path = m_settings.userDataDirectory + QStringLiteral("/customer_id");
    m_customerId = storedCustomerId(path);
    if (m_customerId.isEmpty()) {
        m_customerId = QUuid::createUuid().toString(QUuid::WithoutBraces);
        QDir().mkpath(m_settings.userDataDirectory);
        QSaveFile file(path);
        if (!file.open(QIODevice::WriteOnly) || file.write(m_customerId.toUtf8()) < 0 || !file.commit()) {
            qWarning("Could not store the KLIPY customer id in %s: %s", qPrintable(path), qPrintable(file.errorString()));
        }
    }
    return m_customerId;
}

Klipy::RequestContext KlipyClient::context()
{
    return {m_settings.apiKey, customerId(), m_settings.localeName, m_settings.contentFilter};
}

QNetworkRequest KlipyClient::request(const QUrl &url) const
{
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("KBoard"));
    return request;
}

QNetworkReply *KlipyClient::get(const QUrl &url, const SuccessHandler &onSuccess, const ErrorHandler &onError)
{
    if (m_offline) {
        return nullptr;
    }
    QNetworkReply *reply = m_network->get(request(url));
    connect(reply, &QNetworkReply::finished, this, [reply, onSuccess, onError] {
        const QByteArray body = reply->readAll();
        if (reply->error() == QNetworkReply::OperationCanceledError) {
            return;
        }
        if (reply->error() == QNetworkReply::NoError) {
            onSuccess(body);
            return;
        }
        const QString message = Klipy::errorMessage(body);
        onError(message.isEmpty() ? tr("Could not reach KLIPY: %1").arg(reply->errorString()) : message);
    });
    return reply;
}

void KlipyClient::registerShare(const Klipy::Item &item, const QString &query)
{
    if (m_offline || !isConfigured()) {
        return;
    }
    const Klipy::RequestContext requestContext = context();
    QNetworkRequest shareRequest = request(Klipy::shareUrl(requestContext, Klipy::mediaTypeForItem(item), item.slug));
    shareRequest.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    QNetworkReply *reply = m_network->post(shareRequest, Klipy::shareBody(requestContext, query));
    connect(reply, &QNetworkReply::finished, this, [reply] {
        QString error = reply->errorString();
        if (reply->error() == QNetworkReply::NoError && Klipy::parseShare(reply->readAll(), &error)) {
            return;
        }
        qWarning("KLIPY share trigger failed: %s", qPrintable(error));
    });
}
