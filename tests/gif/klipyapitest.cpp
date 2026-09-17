#include "klipyapi.h"
#include "fixtures.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QTest>
#include <QUrlQuery>

class KlipyApiTest : public QObject
{
    Q_OBJECT

private:
    static Klipy::RequestContext context()
    {
        Klipy::RequestContext context;
        context.apiKey = QStringLiteral("test-key");
        context.customerId = QStringLiteral("c0ffee00-1111-2222-3333-444455556666");
        context.localeName = QStringLiteral("de_DE");
        context.contentFilter = Klipy::ContentFilter::High;
        return context;
    }

private Q_SLOTS:
    void trendingUrl()
    {
        const QUrl url = Klipy::trendingUrl(context(), Klipy::MediaType::Gifs, 3, 30);
        QCOMPARE(url.scheme(), QStringLiteral("https"));
        QCOMPARE(url.host(), QStringLiteral("api.klipy.com"));
        QCOMPARE(url.path(), QStringLiteral("/api/v1/test-key/gifs/trending"));
        const QUrlQuery query(url);
        QCOMPARE(query.queryItemValue(QStringLiteral("page")), QStringLiteral("3"));
        QCOMPARE(query.queryItemValue(QStringLiteral("per_page")), QStringLiteral("30"));
        QCOMPARE(query.queryItemValue(QStringLiteral("customer_id")), context().customerId);
        QCOMPARE(query.queryItemValue(QStringLiteral("locale")), QStringLiteral("de"));
        QCOMPARE(query.queryItemValue(QStringLiteral("content_filter")), QStringLiteral("high"));
        QCOMPARE(query.queryItemValue(QStringLiteral("format_filter")), QStringLiteral("gif,webp"));
    }

    void searchUrlEncodesQuery()
    {
        const QUrl url = Klipy::searchUrl(context(), Klipy::MediaType::Stickers, QStringLiteral("  cats & dogs+1 =#ü "), 1);
        QCOMPARE(url.path(), QStringLiteral("/api/v1/test-key/stickers/search"));
        const QUrlQuery query(url);
        QCOMPARE(query.queryItemValue(QStringLiteral("q"), QUrl::FullyDecoded), QStringLiteral("cats & dogs+1 =#ü"));
        QCOMPARE(query.queryItemValue(QStringLiteral("page")), QStringLiteral("1"));
        QCOMPARE(query.queryItemValue(QStringLiteral("per_page")), QString::number(Klipy::defaultPerPage));
        QVERIFY(url.toEncoded().contains("q=cats%20%26%20dogs%2B1%20%3D%23%C3%BC"));
    }

    void categoriesUrl()
    {
        const QUrl url = Klipy::categoriesUrl(context(), Klipy::MediaType::Gifs);
        QCOMPARE(url.path(), QStringLiteral("/api/v1/test-key/gifs/categories"));
        QCOMPARE(QUrlQuery(url).queryItemValue(QStringLiteral("locale")), QStringLiteral("de_DE"));
    }

    void autocompleteAndSuggestionUrls()
    {
        const QUrl autocomplete = Klipy::autocompleteUrl(context(), QStringLiteral("good morn"), 8);
        QCOMPARE(autocomplete.path(QUrl::FullyEncoded), QStringLiteral("/api/v1/test-key/autocomplete/good%20morn"));
        QCOMPARE(QUrlQuery(autocomplete).queryItemValue(QStringLiteral("limit")), QStringLiteral("8"));
        const QUrl suggestions = Klipy::searchSuggestionsUrl(context(), QStringLiteral("a/b"));
        QCOMPARE(suggestions.path(QUrl::FullyEncoded), QStringLiteral("/api/v1/test-key/search-suggestions/a%2Fb"));
    }

    void shareRequest()
    {
        const QUrl url = Klipy::shareUrl(context(), Klipy::MediaType::Gifs, QStringLiteral("hello-hi-662"));
        QCOMPARE(url.toString(), QStringLiteral("https://api.klipy.com/api/v1/test-key/gifs/share/hello-hi-662"));
        const QJsonObject body = QJsonDocument::fromJson(Klipy::shareBody(context(), QStringLiteral(" hello  there "))).object();
        QCOMPARE(body.value(QStringLiteral("customer_id")).toString(), context().customerId);
        QCOMPARE(body.value(QStringLiteral("q")).toString(), QStringLiteral("hello there"));
        QVERIFY(!QJsonDocument::fromJson(Klipy::shareBody(context(), QString())).object().contains(QStringLiteral("q")));
        QString error;
        QVERIFY(Klipy::parseShare(fixture(QStringLiteral("share.json")), &error));
    }

    void localeMapping()
    {
        QCOMPARE(Klipy::searchLocale(QStringLiteral("en_US")), QStringLiteral("us"));
        QCOMPARE(Klipy::categoryLocale(QStringLiteral("pt_BR")), QStringLiteral("pt_BR"));
        QCOMPARE(Klipy::searchLocale(QStringLiteral("C")), QString());
        QCOMPARE(Klipy::categoryLocale(QStringLiteral("C")), QString());
    }

    void parseTrendingPage()
    {
        Klipy::Page page;
        QString error;
        QVERIFY2(Klipy::parsePage(fixture(QStringLiteral("gifs-trending.json")), &page, &error), qPrintable(error));
        QCOMPARE(page.items.size(), 1);
        QCOMPARE(page.currentPage, 1);
        QCOMPARE(page.perPage, 24);
        QVERIFY(page.hasNext);
        const Klipy::Item &item = page.items.first();
        QCOMPARE(item.id, QStringLiteral("8041071659142944"));
        QCOMPARE(item.slug, QStringLiteral("hello-hi-662"));
        QCOMPARE(item.title, QStringLiteral("Hello"));
        QCOMPARE(item.type, QStringLiteral("gif"));
        QVERIFY(item.blurPreview.startsWith(QStringLiteral("data:image/jpeg;base64,")));
        QVERIFY(item.preview.url.endsWith(QStringLiteral(".webp")));
        QCOMPARE(item.preview.width, 220);
        QCOMPARE(item.preview.height, 220);
        QVERIFY(item.shareGif.url.endsWith(QStringLiteral("8GCrVAB7.gif")));
        QVERIFY(item.linkGif.url.endsWith(QStringLiteral("um0L4dFH.gif")));
    }

    void parseStickerPage()
    {
        Klipy::Page page;
        QString error;
        QVERIFY2(Klipy::parsePage(fixture(QStringLiteral("stickers-search.json")), &page, &error), qPrintable(error));
        QCOMPARE(page.items.size(), 1);
        const Klipy::Item &item = page.items.first();
        QCOMPARE(item.type, QStringLiteral("sticker"));
        QCOMPARE(Klipy::mediaTypeForItem(item), Klipy::MediaType::Stickers);
        QCOMPARE(item.preview.width, 189);
        QCOMPARE(item.preview.height, 200);
        QVERIFY(item.shareGif.url.endsWith(QStringLiteral(".gif")));
    }

    void parseSkipsItemsWithoutMedia()
    {
        const QByteArray json
            = R"({"result":true,"data":{"data":[{"id":1,"slug":"x","type":"ad"},{"id":2,"slug":"y","type":"gif","file":{"sm":{"gif":{"url":"https://static.klipy.com/y.gif","width":10,"height":20}}}}],"current_page":2,"per_page":2,"has_next":false}})";
        Klipy::Page page;
        QString error;
        QVERIFY(Klipy::parsePage(json, &page, &error));
        QCOMPARE(page.items.size(), 1);
        QCOMPARE(page.items.first().id, QStringLiteral("2"));
        QCOMPARE(page.items.first().preview.url, QStringLiteral("https://static.klipy.com/y.gif"));
        QCOMPARE(page.items.first().shareGif.url, QStringLiteral("https://static.klipy.com/y.gif"));
        QVERIFY(page.items.first().linkGif.url.isEmpty());
        QVERIFY(!page.hasNext);
    }

    void parseCategories()
    {
        QList<Klipy::Category> categories;
        QString error;
        QVERIFY2(Klipy::parseCategories(fixture(QStringLiteral("gifs-categories.json")), &categories, &error), qPrintable(error));
        QCOMPARE(categories.size(), 5);
        QCOMPARE(categories.at(2).name, QStringLiteral("high five"));
        QCOMPARE(categories.at(2).query, QStringLiteral("high five"));
        QVERIFY(categories.at(0).previewUrl.startsWith(QStringLiteral("https://static.klipy.com/")));
    }

    void parseAutocomplete()
    {
        QStringList strings;
        QString error;
        QVERIFY2(Klipy::parseStringList(fixture(QStringLiteral("autocomplete.json")), &strings, &error), qPrintable(error));
        QCOMPARE(strings.size(), 30);
        QCOMPARE(strings.first(), QStringLiteral("hello"));
    }

    void parseErrors()
    {
        Klipy::Page page;
        QString error;
        QVERIFY(!Klipy::parsePage(fixture(QStringLiteral("error-invalid-key.json")), &page, &error));
        QCOMPARE(error, QStringLiteral("The provided API key is invalid."));
        QVERIFY(!Klipy::parsePage("<html>502</html>", &page, &error));
        QVERIFY(error.startsWith(QStringLiteral("KLIPY sent an unreadable response")));
        QVERIFY(!Klipy::parsePage(R"({"result":true,"data":{}})", &page, &error));
    }

    void itemJsonRoundTrip()
    {
        Klipy::Page page;
        QString error;
        QVERIFY(Klipy::parsePage(fixture(QStringLiteral("gifs-trending.json")), &page, &error));
        const Klipy::Item original = page.items.first();
        const Klipy::Item copy = Klipy::Item::fromJson(original.toJson());
        QCOMPARE(copy.id, original.id);
        QCOMPARE(copy.preview.url, original.preview.url);
        QCOMPARE(copy.shareGif.size, original.shareGif.size);
        QCOMPARE(copy.linkGif.url, original.linkGif.url);
        QVERIFY(copy.isValid());
    }
};

QTEST_GUILESS_MAIN(KlipyApiTest)

#include "klipyapitest.moc"
