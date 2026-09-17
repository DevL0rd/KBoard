#include "klipyapi.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QLocale>
#include <QUrlQuery>

namespace Klipy
{

namespace
{

QString encodeSegment(const QString &segment)
{
    return QString::fromLatin1(QUrl::toPercentEncoding(segment));
}

QUrl endpoint(const RequestContext &context, const QString &path, const QUrlQuery &query = {})
{
    QUrl url(baseUrl + encodeSegment(context.apiKey) + QLatin1Char('/') + path);
    if (!query.isEmpty()) {
        url.setQuery(query);
    }
    return url;
}

QUrl pagedUrl(const RequestContext &context, const QString &path, int page, int perPage, QUrlQuery query)
{
    query.addQueryItem(QStringLiteral("page"), QString::number(page));
    query.addQueryItem(QStringLiteral("per_page"), QString::number(perPage));
    if (!context.customerId.isEmpty()) {
        query.addQueryItem(QStringLiteral("customer_id"), context.customerId);
    }
    const QString locale = searchLocale(context.localeName);
    if (!locale.isEmpty()) {
        query.addQueryItem(QStringLiteral("locale"), locale);
    }
    query.addQueryItem(QStringLiteral("content_filter"), contentFilterName(context.contentFilter));
    query.addQueryItem(QStringLiteral("format_filter"), QStringLiteral("gif,webp"));
    return endpoint(context, path, query);
}

QUrl termUrl(const RequestContext &context, const QString &path, const QString &text, int limit)
{
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("limit"), QString::number(limit));
    return endpoint(context, path + QLatin1Char('/') + encodeSegment(text.simplified()), query);
}

bool hasTerritory(const QLocale &locale)
{
    return locale != QLocale::c() && locale.territory() != QLocale::AnyTerritory;
}

}

QString mediaPath(MediaType type)
{
    return type == MediaType::Stickers ? QStringLiteral("stickers") : QStringLiteral("gifs");
}

QString contentFilterName(ContentFilter filter)
{
    static const QStringList names {QStringLiteral("off"), QStringLiteral("low"), QStringLiteral("medium"), QStringLiteral("high")};
    return names.at(static_cast<int>(filter));
}

QString searchLocale(const QString &systemLocaleName)
{
    const QLocale locale(systemLocaleName);
    return hasTerritory(locale) ? QLocale::territoryToCode(locale.territory()).toLower() : QString();
}

QString categoryLocale(const QString &systemLocaleName)
{
    const QLocale locale(systemLocaleName);
    if (!hasTerritory(locale)) {
        return {};
    }
    return QLocale::languageToCode(locale.language(), QLocale::ISO639Part1) + QLatin1Char('_')
        + QLocale::territoryToCode(locale.territory());
}

QUrl trendingUrl(const RequestContext &context, MediaType type, int page, int perPage)
{
    return pagedUrl(context, mediaPath(type) + QStringLiteral("/trending"), page, perPage, {});
}

QUrl searchUrl(const RequestContext &context, MediaType type, const QString &query, int page, int perPage)
{
    QUrlQuery params;
    params.addQueryItem(QStringLiteral("q"), encodeSegment(query.simplified()));
    return pagedUrl(context, mediaPath(type) + QStringLiteral("/search"), page, perPage, params);
}

QUrl categoriesUrl(const RequestContext &context, MediaType type)
{
    QUrlQuery query;
    const QString locale = categoryLocale(context.localeName);
    if (!locale.isEmpty()) {
        query.addQueryItem(QStringLiteral("locale"), locale);
    }
    return endpoint(context, mediaPath(type) + QStringLiteral("/categories"), query);
}

QUrl autocompleteUrl(const RequestContext &context, const QString &query, int limit)
{
    return termUrl(context, QStringLiteral("autocomplete"), query, limit);
}

QUrl searchSuggestionsUrl(const RequestContext &context, const QString &query, int limit)
{
    return termUrl(context, QStringLiteral("search-suggestions"), query, limit);
}

QUrl shareUrl(const RequestContext &context, MediaType type, const QString &slug)
{
    return endpoint(context, mediaPath(type) + QStringLiteral("/share/") + encodeSegment(slug));
}

QByteArray shareBody(const RequestContext &context, const QString &query)
{
    QJsonObject body {{QStringLiteral("customer_id"), context.customerId}};
    if (!query.trimmed().isEmpty()) {
        body.insert(QStringLiteral("q"), query.simplified());
    }
    return QJsonDocument(body).toJson(QJsonDocument::Compact);
}

MediaType mediaTypeForItem(const Item &item)
{
    return item.type == QLatin1String("sticker") ? MediaType::Stickers : MediaType::Gifs;
}

}
