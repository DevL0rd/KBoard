#include "gifdiscovery.h"
#include "klipyclient.h"

#include <QNetworkReply>

namespace
{

void abortReply(const QPointer<QNetworkReply> &reply)
{
    if (reply) {
        reply->abort();
    }
}

}

GifDiscovery::GifDiscovery(KlipyClient *client, QObject *parent)
    : QObject(parent)
    , m_client(client)
{ }

QVariantList GifDiscovery::categories() const
{
    return m_categories;
}

QStringList GifDiscovery::suggestions() const
{
    return m_suggestions;
}

void GifDiscovery::fetchCategories(Klipy::MediaType type)
{
    abortReply(m_categoriesReply);
    const auto onSuccess = [this](const QByteArray &body) {
        QList<Klipy::Category> categories;
        QString error;
        if (!Klipy::parseCategories(body, &categories, &error)) {
            Q_EMIT failed(error);
            return;
        }
        setCategories(categories);
    };
    const auto onError = [this](const QString &error) { Q_EMIT failed(error); };
    m_categoriesReply = m_client->get(Klipy::categoriesUrl(m_client->context(), type), onSuccess, onError);
}

void GifDiscovery::fetchSuggestions(const QString &query)
{
    cancelSuggestions();
    const auto onSuccess = [this, query](const QByteArray &body) {
        QStringList strings;
        QString error;
        if (Klipy::parseStringList(body, &strings, &error)) {
            strings.removeAll(query);
            setSuggestions(strings);
        }
    };
    const auto onError = [](const QString &error) { qWarning("KLIPY autocomplete failed: %s", qPrintable(error)); };
    m_suggestionsReply = m_client->get(Klipy::autocompleteUrl(m_client->context(), query), onSuccess, onError);
}

void GifDiscovery::cancelSuggestions()
{
    abortReply(m_suggestionsReply);
}

void GifDiscovery::setCategories(const QList<Klipy::Category> &categories)
{
    m_categories.clear();
    for (const Klipy::Category &category : categories) {
        m_categories.append(QVariantMap {{QStringLiteral("name"), category.name}, {QStringLiteral("query"), category.query},
            {QStringLiteral("previewUrl"), category.previewUrl}});
    }
    Q_EMIT categoriesChanged();
}

void GifDiscovery::setSuggestions(const QStringList &suggestions)
{
    if (m_suggestions != suggestions) {
        m_suggestions = suggestions;
        Q_EMIT suggestionsChanged();
    }
}
