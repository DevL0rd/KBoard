#include "settingscatalog.h"

#include <QDirIterator>
#include <QFile>
#include <QRegularExpression>
#include <QSet>
#include <QTest>
#include <QXmlStreamReader>
#include <algorithm>
#include <iterator>

namespace
{
const QString SourceDir = QStringLiteral(KBOARD_SETTINGS_SOURCE_DIR);

QStringList kcfgEntries()
{
    QFile file(QStringLiteral(KBOARD_KCFG_PATH));
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }
    QStringList names;
    QXmlStreamReader xml(&file);
    while (!xml.atEnd()) {
        if (xml.readNext() == QXmlStreamReader::StartElement && xml.name() == QLatin1String("entry")) {
            names.append(xml.attributes().value(QStringLiteral("name")).toString());
        }
    }
    return names;
}

QString pageSources(const QString &pageId)
{
    QString text;
    QDirIterator it(SourceDir + QStringLiteral("/qml/pages/") + pageId, {QStringLiteral("*.qml")}, QDir::Files);
    while (it.hasNext()) {
        QFile file(it.next());
        if (file.open(QIODevice::ReadOnly)) {
            text += QString::fromUtf8(file.readAll());
        }
    }
    return text;
}

QStringList captures(const QString &text, const QRegularExpression &pattern)
{
    QStringList result;
    auto matches = pattern.globalMatch(text);
    while (matches.hasNext()) {
        result.append(matches.next().captured(1));
    }
    return result;
}
}

class SettingsIndexTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void everyKcfgEntryIsIndexed();
    void indexedSettingsExist();
    void pagesHaveSources();
    void indexedLabelsExistOnTheirPage();
    void everyRowIsIndexed();
    void searchFindsRows();

private:
    SettingsCatalog catalog {SourceDir + QStringLiteral("/catalog/index.json")};
    QStringList kcfg;
};

void SettingsIndexTest::initTestCase()
{
    QVERIFY2(catalog.errorString().isEmpty(), qPrintable(catalog.errorString()));
    kcfg = kcfgEntries();
    QVERIFY(kcfg.size() > 50);
}

void SettingsIndexTest::everyKcfgEntryIsIndexed()
{
    QSet<QString> indexed;
    for (const QVariant &entry : catalog.entries()) {
        const QStringList settings = entry.toMap().value(QStringLiteral("settings")).toStringList();
        indexed.unite(QSet<QString>(settings.cbegin(), settings.cend()));
    }
    QStringList missing;
    for (const QString &name : std::as_const(kcfg)) {
        if (!indexed.contains(name)) {
            missing.append(name);
        }
    }
    QVERIFY2(missing.isEmpty(), qPrintable(QStringLiteral("Not in the settings index: ") + missing.join(QStringLiteral(", "))));
}

void SettingsIndexTest::indexedSettingsExist()
{
    QStringList unknown;
    const QVariantList entries = catalog.entries();
    for (const QVariant &entry : entries) {
        const QStringList settings = entry.toMap().value(QStringLiteral("settings")).toStringList();
        std::copy_if(
            settings.cbegin(), settings.cend(), std::back_inserter(unknown), [this](const QString &name) { return !kcfg.contains(name); });
    }
    QVERIFY2(unknown.isEmpty(), qPrintable(QStringLiteral("Index refers to unknown settings: ") + unknown.join(QStringLiteral(", "))));
}

void SettingsIndexTest::pagesHaveSources()
{
    for (const QVariant &page : catalog.pages()) {
        const QVariantMap map = page.toMap();
        const QString path = SourceDir
            + QStringLiteral("/qml/pages/%1/%2.qml")
                  .arg(map.value(QStringLiteral("id")).toString(), map.value(QStringLiteral("type")).toString());
        QVERIFY2(QFile::exists(path), qPrintable(path));
    }
    for (const QVariant &entry : catalog.entries()) {
        QVERIFY(catalog.hasPage(entry.toMap().value(QStringLiteral("page")).toString()));
    }
}

void SettingsIndexTest::indexedLabelsExistOnTheirPage()
{
    for (const QVariant &value : catalog.entries()) {
        const QVariantMap entry = value.toMap();
        const QString page = entry.value(QStringLiteral("page")).toString();
        const QString label = entry.value(QStringLiteral("label")).toString();
        const QString needle = QStringLiteral("label: \"%1\"").arg(label);
        QVERIFY2(pageSources(page).contains(needle), qPrintable(QStringLiteral("%1 has no row labelled \"%2\"").arg(page, label)));
    }
}

void SettingsIndexTest::everyRowIsIndexed()
{
    static const QRegularExpression rowLabel(QStringLiteral("\\b[A-Z]\\w*(?:Row|Card)\\s*\\{[^{}]*?\\blabel:\\s*\"([^\"]+)\""));
    QStringList missing;
    const QVariantList pages = catalog.pages();
    for (const QVariant &page : pages) {
        const QString pageId = page.toMap().value(QStringLiteral("id")).toString();
        QStringList indexed;
        const QVariantList entries = catalog.entriesForPage(pageId);
        for (const QVariant &entry : entries) {
            indexed.append(entry.toMap().value(QStringLiteral("label")).toString());
        }
        const QStringList labels = captures(pageSources(pageId), rowLabel);
        for (const QString &label : labels) {
            if (!indexed.contains(label)) {
                missing.append(pageId + QLatin1Char('/') + label);
            }
        }
    }
    QVERIFY2(missing.isEmpty(), qPrintable(QStringLiteral("Rows missing from the settings index: ") + missing.join(QStringLiteral(", "))));
}

void SettingsIndexTest::searchFindsRows()
{
    const QVariantList results = catalog.search(QStringLiteral("split"));
    QVERIFY(!results.isEmpty());
    QCOMPARE(results.constFirst().toMap().value(QStringLiteral("page")).toString(), QStringLiteral("layout"));
    QVERIFY(catalog.search(QStringLiteral("zzzz-not-a-setting")).isEmpty());
    QVERIFY(catalog.search(QString()).isEmpty());
}

QTEST_GUILESS_MAIN(SettingsIndexTest)

#include "tst_settingsindex.moc"
