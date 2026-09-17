#include "gifstore.h"
#include "fixtures.h"
#include "gifmodel.h"
#include "gifpaste.h"
#include "klipyclient.h"

#include <QDateTime>
#include <QFile>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTest>
#include <QUuid>

class GifStoreTest : public QObject
{
    Q_OBJECT

private:
    static GifStore::Options options(const QTemporaryDir &dir, const QString &key)
    {
        return {key, dir.filePath(QStringLiteral("data")), dir.filePath(QStringLiteral("cache")), QStringLiteral("en_US")};
    }

private Q_SLOTS:
    void initTestCase() { QStandardPaths::setTestModeEnabled(true); }

    void notConfiguredStore()
    {
        QTemporaryDir dir;
        GifStore store(options(dir, QString()));
        QVERIFY(!store.isConfigured());
        QCOMPARE(store.errorString(), QStringLiteral("GIFs need a KLIPY API key at build time"));
        store.load();
        store.loadMore();
        store.setQuery(QStringLiteral("cat"));
        QVERIFY(!store.isLoading());
        QCOMPARE(store.trending()->rowCount(), 0);
        store.clearError();
        QCOMPARE(store.errorString(), QStringLiteral("GIFs need a KLIPY API key at build time"));
    }

    void configuredStoreStartsClean()
    {
        QTemporaryDir dir;
        GifStore store(options(dir, QStringLiteral("key")));
        QVERIFY(store.isConfigured());
        QVERIFY(store.errorString().isEmpty());
        QCOMPARE(store.searchPlaceholder(), QStringLiteral("Search KLIPY"));
        QCOMPARE(GifStore::debounceInterval(), 250);
        const QString id = store.client()->customerId();
        QVERIFY(!QUuid::fromString(id).isNull());
        GifStore again(options(dir, QStringLiteral("key")));
        QCOMPARE(again.client()->customerId(), id);
    }

    void fixturesFavoritesAndRecents()
    {
        QTemporaryDir dir;
        QTemporaryDir fixtures;
        QFile::copy(fixturePath(QStringLiteral("gifs-trending.json")), fixtures.filePath(QStringLiteral("trending.json")));
        QFile::copy(fixturePath(QStringLiteral("stickers-search.json")), fixtures.filePath(QStringLiteral("search.json")));
        QFile::copy(fixturePath(QStringLiteral("gifs-categories.json")), fixtures.filePath(QStringLiteral("categories.json")));
        GifStore store(options(dir, QString()));
        QSignalSpy configured(&store, &GifStore::configuredChanged);
        QVERIFY(store.loadFixtures(fixtures.path()));
        QCOMPARE(configured.count(), 1);
        QVERIFY(store.isConfigured());
        QVERIFY(store.errorString().isEmpty());
        QCOMPARE(store.trending()->rowCount(), 1);
        QCOMPARE(store.results()->rowCount(), 1);
        QCOMPARE(store.categories().size(), 5);

        const QString id = QStringLiteral("8041071659142944");
        store.toggleFavorite(id);
        QVERIFY(store.isFavorite(id));
        QCOMPARE(store.favorites()->rowCount(), 1);
        QVERIFY(store.trending()->data(store.trending()->index(0), GifModel::FavoriteRole).toBool());

        GifStore reloaded(options(dir, QString()));
        QCOMPARE(reloaded.favorites()->rowCount(), 1);
        QVERIFY(reloaded.isFavorite(id));

        store.toggleFavorite(id);
        QVERIFY(!store.isFavorite(id));
        GifStore cleared(options(dir, QString()));
        QCOMPARE(cleared.favorites()->rowCount(), 0);
    }

    void masonryLayout()
    {
        GifModel model;
        QList<Klipy::Item> items;
        const QList<QSize> sizes {{200, 200}, {200, 400}, {200, 100}, {200, 200}, {200, 200}};
        for (int i = 0; i < sizes.size(); ++i) {
            Klipy::Item item;
            item.id = QString::number(i);
            item.slug = item.id;
            item.preview = {QStringLiteral("https://x/%1.webp").arg(i), sizes.at(i).width(), sizes.at(i).height(), 1};
            item.shareGif = item.preview;
            items.append(item);
        }
        model.setColumns(2);
        model.setSpacing(10);
        model.setLayoutWidth(210);
        model.setItems(items);
        auto tile = [&model](int row) {
            const QModelIndex index = model.index(row);
            return QRectF(model.data(index, GifModel::TileXRole).toReal(), model.data(index, GifModel::TileYRole).toReal(),
                model.data(index, GifModel::TileWidthRole).toReal(), model.data(index, GifModel::TileHeightRole).toReal());
        };
        QCOMPARE(tile(0), QRectF(0, 0, 100, 100));
        QCOMPARE(tile(1), QRectF(110, 0, 100, 200));
        QCOMPARE(tile(2), QRectF(0, 110, 100, 50));
        QCOMPARE(tile(3), QRectF(0, 170, 100, 100));
        QCOMPARE(tile(4), QRectF(110, 210, 100, 100));
        QCOMPARE(model.contentHeight(), 310.0);
        Klipy::Item extra = items.first();
        extra.id = QStringLiteral("extra");
        model.appendItems({extra, items.first()});
        QCOMPARE(model.rowCount(), 6);
        QCOMPARE(tile(5), QRectF(0, 280, 100, 100));
    }

    void mimeData()
    {
        const QByteArray gif("GIF89a\x01\x00\x01\x00\x00\x00\x00;", 14);
        std::unique_ptr<QMimeData> mime(GifPaste::buildMimeData(gif, QStringLiteral("/home/user/.cache/kboard/gif/hello world.gif")));
        QCOMPARE(mime->data(QStringLiteral("image/gif")), gif);
        QCOMPARE(mime->data(QStringLiteral("text/uri-list")), QByteArray("file:///home/user/.cache/kboard/gif/hello%20world.gif\r\n"));
        QCOMPARE(mime->data(QStringLiteral("x-special/gnome-copied-files")),
            QByteArray("copy\nfile:///home/user/.cache/kboard/gif/hello%20world.gif"));
        QCOMPARE(mime->formats(),
            QStringList({QStringLiteral("image/gif"), QStringLiteral("text/uri-list"), QStringLiteral("x-special/gnome-copied-files")}));
        QVERIFY(!mime->hasFormat(QStringLiteral("text/plain")));
        QCOMPARE(mime->urls().size(), 1);
        QVERIFY(GifPaste::isGif(gif));
        QVERIFY(!GifPaste::isGif("\x89PNG"));
    }

    void cacheNamesAndTrim()
    {
        Klipy::Item item;
        item.slug = QStringLiteral("hello/../hi 662");
        item.shareGif.url = QStringLiteral("https://static.klipy.com/a.gif");
        const QString name = GifPaste::cacheFileName(item);
        QVERIFY(name.startsWith(QStringLiteral("hello----hi-662-")));
        QVERIFY(name.endsWith(QStringLiteral(".gif")));
        QVERIFY(!name.contains(QLatin1Char('/')));

        QTemporaryDir dir;
        const QByteArray bytes(1000, 'x');
        QString error;
        const QDateTime now = QDateTime::currentDateTimeUtc();
        for (int i = 0; i < 5; ++i) {
            const QString path = dir.filePath(QStringLiteral("%1.gif").arg(i));
            QVERIFY(GifPaste::writeCacheFile(path, bytes, &error));
            QFile file(path);
            QVERIFY(file.open(QIODevice::ReadWrite));
            QVERIFY(file.setFileTime(now.addSecs(qint64(i) * 10), QFileDevice::FileModificationTime));
        }
        const qint64 total = GifPaste::trimCache(dir.path(), 2500, dir.filePath(QStringLiteral("0.gif")));
        QCOMPARE(total, 2000);
        QVERIFY(QFile::exists(dir.filePath(QStringLiteral("0.gif"))));
        QVERIFY(QFile::exists(dir.filePath(QStringLiteral("4.gif"))));
        QVERIFY(!QFile::exists(dir.filePath(QStringLiteral("3.gif"))));
        QVERIFY(!QFile::exists(dir.filePath(QStringLiteral("1.gif"))));
    }
};

QTEST_GUILESS_MAIN(GifStoreTest)

#include "gifstoretest.moc"
