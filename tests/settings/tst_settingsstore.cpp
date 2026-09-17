#include "edithistory.h"
#include "settingsstore.h"

#include <KConfig>
#include <KConfigGroup>

#include <QColor>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTest>

class SettingsStoreTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void init();
    void historyCoalescesBursts();
    void historyGroupsSteps();
    void undoRestoresEdits();
    void undoRestoresDefaults();
    void rejectsUnknownSettings();
    void clampsToRange();
    void exportImportRoundTrip();
    void importRejectsForeignFiles();

private:
    SettingsStore *store = nullptr;
    QTemporaryDir dir;
    int baseline = 0;
};

void SettingsStoreTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    QFile::remove(QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QStringLiteral("/kboardrc"));
    store = SettingsStore::instance();
    QVERIFY(dir.isValid());
}

void SettingsStoreTest::init()
{
    store->defaults();
    store->flush();
    baseline = store->undoCount();
}

void SettingsStoreTest::historyCoalescesBursts()
{
    EditHistory history;
    history.record(QStringLiteral("keyGap"), 6, 1000);
    history.record(QStringLiteral("keyGap"), 7, 1200);
    history.record(QStringLiteral("keyGap"), 8, 1500);
    QCOMPARE(history.size(), 1);
    history.record(QStringLiteral("keyGap"), 9, 1500 + EditHistory::BurstMs + 1);
    QCOMPARE(history.size(), 2);
    history.record(QStringLiteral("keyRadius"), 10, 1500 + EditHistory::BurstMs + 2);
    QCOMPARE(history.size(), 3);
    const auto step = history.undo();
    QVERIFY(step.has_value());
    QCOMPARE(step->constFirst().name, QStringLiteral("keyRadius"));
    const auto older = history.undo();
    QCOMPARE(older->constFirst().before.toInt(), 9);
    QCOMPARE(history.undo()->constFirst().before.toInt(), 6);
    QVERIFY(!history.canUndo());
    QVERIFY(!history.undo().has_value());
}

void SettingsStoreTest::historyGroupsSteps()
{
    EditHistory history;
    history.record(QStringLiteral("keyGap"), 6, 0);
    history.recordStep({{QStringLiteral("keyGap"), 8}, {QStringLiteral("keyRadius"), 12}});
    history.record(QStringLiteral("keyGap"), 3, 10);
    QCOMPARE(history.size(), 3);
    history.recordStep({});
    QCOMPARE(history.size(), 3);
    history.undo();
    QCOMPARE(history.undo()->size(), 2);
}

void SettingsStoreTest::undoRestoresEdits()
{
    QVERIFY(store->setValue(QStringLiteral("soundVolume"), 0.25));
    QVERIFY(store->setValue(QStringLiteral("keyStyle"), 3));
    QVERIFY(!store->isDefault(QStringLiteral("soundVolume")));
    QCOMPARE(store->undoCount(), baseline + 2);
    QVERIFY(store->undo());
    QCOMPARE(store->value(QStringLiteral("keyStyle")).toInt(), store->defaultValue(QStringLiteral("keyStyle")).toInt());
    QCOMPARE(store->value(QStringLiteral("soundVolume")).toDouble(), 0.25);
    QVERIFY(store->undo());
    QVERIFY(store->isDefault(QStringLiteral("soundVolume")));
    QCOMPARE(store->undoCount(), baseline);

    QVERIFY(store->setValues({{QStringLiteral("layouts"), QStringList {QStringLiteral("us"), QStringLiteral("fr")}},
        {QStringLiteral("activeLayout"), QStringLiteral("fr")}}));
    QCOMPARE(store->undoCount(), baseline + 1);
    QCOMPARE(store->value(QStringLiteral("activeLayout")).toString(), QStringLiteral("fr"));
    QVERIFY(!store->setValues({{QStringLiteral("activeLayout"), QStringLiteral("de")}, {QStringLiteral("bogus"), 1}}));
    QCOMPARE(store->value(QStringLiteral("activeLayout")).toString(), QStringLiteral("fr"));
    QVERIFY(store->undo());
    QVERIFY(store->isDefault(QStringLiteral("layouts")));
    QVERIFY(store->isDefault(QStringLiteral("activeLayout")));
}

void SettingsStoreTest::undoRestoresDefaults()
{
    store->setValue(QStringLiteral("keyRadius"), 20);
    store->setValue(QStringLiteral("textExpansions"), QStringList {QStringLiteral("gm=Good morning")});
    store->setValue(QStringLiteral("accentColor"), QStringLiteral("#ff0080"));
    QVERIFY(!store->representsDefaults());
    store->defaults();
    QVERIFY(store->representsDefaults());
    QVERIFY(store->undo());
    QCOMPARE(store->value(QStringLiteral("keyRadius")).toInt(), 20);
    QCOMPARE(store->value(QStringLiteral("textExpansions")).toStringList(), QStringList {QStringLiteral("gm=Good morning")});
    QCOMPARE(store->value(QStringLiteral("accentColor")).value<QColor>(), QColor(QStringLiteral("#ff0080")));
    QVERIFY(store->resetToDefault(QStringLiteral("keyRadius")));
    QVERIFY(store->isDefault(QStringLiteral("keyRadius")));
    QVERIFY(store->undo());
    QCOMPARE(store->value(QStringLiteral("keyRadius")).toInt(), 20);
}

void SettingsStoreTest::rejectsUnknownSettings()
{
    QSignalSpy failed(store, &SettingsStore::failed);
    QVERIFY(!store->setValue(QStringLiteral("noSuchSetting"), 1));
    QVERIFY(!store->resetToDefault(QStringLiteral("noSuchSetting")));
    QCOMPARE(failed.count(), 2);
    QCOMPARE(store->undoCount(), baseline);
}

void SettingsStoreTest::clampsToRange()
{
    QVERIFY(store->setValue(QStringLiteral("keyGap"), 99));
    QCOMPARE(store->value(QStringLiteral("keyGap")).toInt(), 20);
    QVERIFY(store->setValue(QStringLiteral("heightLandscape"), 0.01));
    QCOMPARE(store->value(QStringLiteral("heightLandscape")).toDouble(), 0.2);
}

void SettingsStoreTest::exportImportRoundTrip()
{
    store->setValue(QStringLiteral("keyStyle"), 3);
    store->setValue(QStringLiteral("backgroundOpacity"), 0.5);
    store->setValue(QStringLiteral("glideTrail"), false);
    store->setValue(QStringLiteral("accentColor"), QStringLiteral("#12ab34"));
    store->setValue(QStringLiteral("layouts"), QStringList {QStringLiteral("us"), QStringLiteral("de")});
    store->setValue(QStringLiteral("soundPack"), QStringLiteral("typewriter"));
    QVariantMap expected;
    for (const QString &name : store->entryNames()) {
        expected.insert(name, store->value(name));
    }
    const QString path = dir.filePath(QStringLiteral("roundtrip.kboardsettings"));
    QString error;
    QVERIFY2(store->exportToFile(path, &error), qPrintable(error));
    QVERIFY(KConfig(path, KConfig::SimpleConfig).groupList().contains(QStringLiteral("Look")));

    store->defaults();
    QVERIFY(store->representsDefaults());
    QVERIFY2(store->importFromFile(path, &error), qPrintable(error));
    for (auto it = expected.cbegin(); it != expected.cend(); ++it) {
        QCOMPARE(store->value(it.key()), it.value());
    }
    QVERIFY(store->undo());
    QVERIFY(store->representsDefaults());
}

void SettingsStoreTest::importRejectsForeignFiles()
{
    const QString path = dir.filePath(QStringLiteral("foreign.rc"));
    {
        KConfig foreign(path, KConfig::SimpleConfig);
        foreign.group(QStringLiteral("Something")).writeEntry("else", 1);
        foreign.sync();
    }
    QString error;
    QVERIFY(!store->importFromFile(path, &error));
    QVERIFY(error.contains(QStringLiteral("doesn't contain KBoard settings")));
    QVERIFY(!store->importFromFile(dir.filePath(QStringLiteral("missing.rc")), &error));
    QVERIFY(store->representsDefaults());
}

QTEST_GUILESS_MAIN(SettingsStoreTest)

#include "tst_settingsstore.moc"
