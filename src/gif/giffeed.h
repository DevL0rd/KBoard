#pragma once

#include "gifmodel.h"

#include <QObject>
#include <QPointer>
#include <functional>

class KlipyClient;
class QNetworkReply;

class GifFeed : public QObject
{
    Q_OBJECT

public:
    using UrlFactory = std::function<QUrl(int page)>;

    GifFeed(KlipyClient *client, QObject *parent = nullptr);

    GifModel *model() const;
    bool isBusy() const;
    bool isEmpty() const;
    void reload(const UrlFactory &urlFactory);
    void loadMore();
    void cancel();
    void clear();

Q_SIGNALS:
    void busyChanged();
    void failed(const QString &error);
    void loaded();

private:
    void fetch(int page);

    KlipyClient *m_client;
    GifModel *m_model;
    UrlFactory m_urlFactory;
    QPointer<QNetworkReply> m_reply;
    int m_page = 0;
};
