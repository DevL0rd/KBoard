#pragma once

#include "klipyapi.h"

#include <QNetworkAccessManager>
#include <QObject>
#include <QPointer>
#include <functional>

class QNetworkReply;

class KlipyClient : public QObject
{
    Q_OBJECT

public:
    using SuccessHandler = std::function<void(const QByteArray &body)>;
    using ErrorHandler = std::function<void(const QString &error)>;

    struct Settings
    {
        QString apiKey;
        QString userDataDirectory;
        QString localeName;
        Klipy::ContentFilter contentFilter = Klipy::ContentFilter::Medium;
    };

    explicit KlipyClient(Settings settings, QObject *parent = nullptr);

    bool isConfigured() const;
    bool isOffline() const;
    void setOffline(bool offline);
    void setContentFilter(Klipy::ContentFilter filter);
    Klipy::ContentFilter contentFilter() const;
    QString customerId();
    Klipy::RequestContext context();

    QNetworkReply *get(const QUrl &url, const SuccessHandler &onSuccess, const ErrorHandler &onError);
    void registerShare(const Klipy::Item &item, const QString &query);

private:
    QNetworkRequest request(const QUrl &url) const;

    Settings m_settings;
    QNetworkAccessManager *m_network;
    QString m_customerId;
    bool m_offline = false;
};
