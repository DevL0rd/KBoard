#include "giffeed.h"
#include "klipyclient.h"

#include <QNetworkReply>

GifFeed::GifFeed(KlipyClient *client, QObject *parent)
    : QObject(parent)
    , m_client(client)
    , m_model(new GifModel(this))
{ }

GifModel *GifFeed::model() const
{
    return m_model;
}

bool GifFeed::isBusy() const
{
    return !m_reply.isNull();
}

bool GifFeed::isEmpty() const
{
    return m_model->rowCount() == 0;
}

void GifFeed::reload(const UrlFactory &urlFactory)
{
    cancel();
    m_urlFactory = urlFactory;
    m_page = 0;
    fetch(1);
}

void GifFeed::loadMore()
{
    if (!isBusy() && m_urlFactory && m_page > 0 && m_model->hasMore()) {
        fetch(m_page + 1);
    }
}

void GifFeed::cancel()
{
    if (m_reply) {
        m_reply->abort();
    }
}

void GifFeed::clear()
{
    cancel();
    m_urlFactory = nullptr;
    m_page = 0;
    m_model->clear();
}

void GifFeed::fetch(int page)
{
    auto onSuccess = [this, page](const QByteArray &body) {
        Klipy::Page result;
        QString error;
        if (!Klipy::parsePage(body, &result, &error)) {
            Q_EMIT failed(error);
            return;
        }
        m_page = page;
        if (page == 1) {
            m_model->setItems(result.items);
        } else {
            m_model->appendItems(result.items);
        }
        m_model->setHasMore(result.hasNext);
        Q_EMIT loaded();
    };
    QNetworkReply *reply = m_client->get(m_urlFactory(page), onSuccess, [this](const QString &error) { Q_EMIT failed(error); });
    if (!reply) {
        return;
    }
    m_reply = reply;
    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        if (m_reply == reply) {
            m_reply = nullptr;
            Q_EMIT busyChanged();
        }
    });
    Q_EMIT busyChanged();
}
