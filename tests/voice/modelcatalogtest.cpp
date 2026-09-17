#include "modelcatalog.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QTest>

class ModelCatalogTest : public QObject
{
    Q_OBJECT

private:
    static QByteArray bundled()
    {
        QFile file(QStringLiteral(KBOARD_DATA_BUILD_DIR "/voice/models.json"));
        return file.open(QIODevice::ReadOnly) ? file.readAll() : QByteArray();
    }

    static QJsonObject bundledObject() { return QJsonDocument::fromJson(bundled()).object(); }

    static QString errorFor(const QJsonObject &root)
    {
        QString error;
        const ModelCatalog catalog = ModelCatalog::fromJson(QJsonDocument(root).toJson(), &error);
        return catalog.isValid() ? QString() : error;
    }

private Q_SLOTS:
    void parsesBundledCatalogue()
    {
        QString error;
        const ModelCatalog catalog = ModelCatalog::fromJson(bundled(), &error);
        QVERIFY2(catalog.isValid(), qPrintable(error));
        QCOMPARE(catalog.defaultModelId(), QStringLiteral("parakeet-tdt-0.6b-v3-q4_0"));
        QCOMPARE(catalog.models().size(), 5);
        const ModelEntry *parakeet = catalog.find(catalog.defaultModelId());
        QVERIFY(parakeet);
        QCOMPARE(parakeet->engine, QStringLiteral("parakeet"));
        QVERIFY(parakeet->recommended);
        QCOMPARE(parakeet->languages.size(), 25);
        QVERIFY(parakeet->languages.contains(QStringLiteral("en")));
        QCOMPARE(catalog.vad().engine, QStringLiteral("silero-vad"));
        QVERIFY(catalog.find(catalog.vad().id));
    }

    void everyModelIsDownloadable()
    {
        QString error;
        const ModelCatalog catalog = ModelCatalog::fromJson(bundled(), &error);
        QList<ModelEntry> all = catalog.models();
        all.append(catalog.vad());
        for (const ModelEntry &entry : std::as_const(all)) {
            QVERIFY2(entry.url.startsWith(QStringLiteral("https://huggingface.co/")), qPrintable(entry.id));
            QVERIFY2(entry.url.endsWith(entry.file), qPrintable(entry.id));
            QCOMPARE(entry.sha256.size(), 64);
            QVERIFY(entry.sizeBytes > 100000);
        }
        const QVariantMap map = catalog.models().first().toVariantMap();
        QCOMPARE(map.value(QStringLiteral("sizeMb")).toLongLong(), 339);
        QVERIFY(map.value(QStringLiteral("recommended")).toBool());
    }

    void rejectsBrokenCatalogues_data()
    {
        QTest::addColumn<QString>("field");
        QTest::addColumn<QString>("value");
        QTest::addColumn<QString>("expected");
        QTest::newRow("unknown default") << QStringLiteral("defaultModel") << QStringLiteral("nope") << QStringLiteral("default model");
        QTest::newRow("bad engine") << QStringLiteral("engine") << QStringLiteral("vosk") << QStringLiteral("unknown engine");
        QTest::newRow("plain http") << QStringLiteral("url") << QStringLiteral("http://example.com/x.bin") << QStringLiteral("https");
        QTest::newRow("bad hash") << QStringLiteral("sha256") << QStringLiteral("abc") << QStringLiteral("sha256");
        QTest::newRow("path in file") << QStringLiteral("file") << QStringLiteral("../x.bin") << QStringLiteral("file name");
        QTest::newRow("unknown language set") << QStringLiteral("languageSet") << QStringLiteral("klingon")
                                              << QStringLiteral("language set");
    }

    void rejectsBrokenCatalogues()
    {
        QFETCH(QString, field);
        QFETCH(QString, value);
        QFETCH(QString, expected);
        QJsonObject root = bundledObject();
        if (field == u"defaultModel") {
            root.insert(field, value);
        } else {
            QJsonArray models = root.value(u"models").toArray();
            QJsonObject first = models.first().toObject();
            first.insert(field, value);
            models.replace(0, first);
            root.insert(QStringLiteral("models"), models);
        }
        const QString error = errorFor(root);
        QVERIFY2(error.contains(expected), qPrintable(error));
    }

    void rejectsDuplicatesAndGarbage()
    {
        QJsonObject root = bundledObject();
        QJsonArray models = root.value(u"models").toArray();
        models.append(models.first());
        root.insert(QStringLiteral("models"), models);
        QVERIFY(errorFor(root).contains(QStringLiteral("twice")));

        QString error;
        QVERIFY(!ModelCatalog::fromJson("{ not json", &error).isValid());
        QVERIFY(error.contains(QStringLiteral("not valid JSON")));
        QVERIFY(!ModelCatalog::fromFile(QStringLiteral("/nonexistent/models.json"), &error).isValid());
        QVERIFY(error.contains(QStringLiteral("cannot be read")));
    }

    void downloadedNeedsExactSize()
    {
        QTemporaryDir dir;
        ModelEntry entry;
        entry.file = QStringLiteral("model.bin");
        entry.sizeBytes = 4;
        QVERIFY(!ModelCatalog::isDownloaded(entry, dir.path()));
        QFile file(ModelCatalog::pathFor(entry, dir.path()));
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("abc");
        file.close();
        QVERIFY(!ModelCatalog::isDownloaded(entry, dir.path()));
        QVERIFY(file.open(QIODevice::Append));
        file.write("d");
        file.close();
        QVERIFY(ModelCatalog::isDownloaded(entry, dir.path()));
    }
};

QTEST_GUILESS_MAIN(ModelCatalogTest)
#include "modelcatalogtest.moc"
