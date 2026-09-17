#pragma once

#include <QByteArray>
#include <QJsonObject>
#include <QList>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QVariantMap>

namespace Klipy
{

enum class MediaType
{
    Gifs,
    Stickers,
};

enum class ContentFilter
{
    Off,
    Low,
    Medium,
    High,
};

struct Rendition
{
    QString url;
    int width = 0;
    int height = 0;
    qint64 size = 0;
};

struct Item
{
    QString id;
    QString slug;
    QString title;
    QString type;
    QString blurPreview;
    Rendition preview;
    Rendition shareGif;
    Rendition linkGif;

    bool isValid() const;
    QJsonObject toJson() const;
    static Item fromJson(const QJsonObject &object);
};

struct Category
{
    QString name;
    QString query;
    QString previewUrl;
};

struct Page
{
    QList<Item> items;
    int currentPage = 0;
    int perPage = 0;
    bool hasNext = false;
};

inline const QString baseUrl = QStringLiteral("https://api.klipy.com/api/v1/");
inline const int defaultPerPage = 24;

QString mediaPath(MediaType type);
QString contentFilterName(ContentFilter filter);
QString searchLocale(const QString &systemLocaleName);
QString categoryLocale(const QString &systemLocaleName);

struct RequestContext
{
    QString apiKey;
    QString customerId;
    QString localeName;
    ContentFilter contentFilter = ContentFilter::Medium;
};

QUrl trendingUrl(const RequestContext &context, MediaType type, int page, int perPage = defaultPerPage);
QUrl searchUrl(const RequestContext &context, MediaType type, const QString &query, int page, int perPage = defaultPerPage);
QUrl categoriesUrl(const RequestContext &context, MediaType type);
QUrl autocompleteUrl(const RequestContext &context, const QString &query, int limit = 10);
QUrl searchSuggestionsUrl(const RequestContext &context, const QString &query, int limit = 10);
QUrl shareUrl(const RequestContext &context, MediaType type, const QString &slug);
QByteArray shareBody(const RequestContext &context, const QString &query);

MediaType mediaTypeForItem(const Item &item);

bool parsePage(const QByteArray &json, Page *page, QString *error);
bool parseCategories(const QByteArray &json, QList<Category> *categories, QString *error);
bool parseStringList(const QByteArray &json, QStringList *strings, QString *error);
bool parseShare(const QByteArray &json, QString *error);
QString errorMessage(const QByteArray &json);
Item parseItem(const QJsonObject &object);

}
