#include "gamepad.h"
#include "kboardsettings.h"

#include <QDir>
#include <QFile>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTest>

namespace
{
qint64 contextSwitches(const QString &threadName)
{
    const QStringList tasks = QDir(QStringLiteral("/proc/self/task")).entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString &task : tasks) {
        QFile comm(QStringLiteral("/proc/self/task/%1/comm").arg(task));
        if (!comm.open(QIODevice::ReadOnly) || QString::fromUtf8(comm.readAll()).trimmed() != threadName) {
            continue;
        }
        QFile status(QStringLiteral("/proc/self/task/%1/status").arg(task));
        if (!status.open(QIODevice::ReadOnly | QIODevice::Text)) {
            return -1;
        }
        qint64 total = 0;
        for (const QByteArray &line : status.readAll().split('\n')) {
            if (line.contains("ctxt_switches")) {
                total += line.split('\t').constLast().trimmed().toLongLong();
            }
        }
        return total;
    }
    return -1;
}
}

class GamepadBackendTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);
        qputenv("KBOARD_USE_BUILD_TREE", "1");
        KBoardSettings::self()->setDefaults();
    }

    void startsAndStopsWithSetting()
    {
        Gamepad gamepad(Gamepad::Mode::WithBackend);
        QTRY_VERIFY_WITH_TIMEOUT(gamepad.available() || !gamepad.errorString().isEmpty(), 5000);
        QVERIFY2(gamepad.available(), qPrintable(gamepad.errorString()));
        QVERIFY2(gamepad.warnings().isEmpty(), qPrintable(gamepad.warnings().join(QLatin1Char('\n'))));

        if (!gamepad.connected()) {
            const qint64 before = contextSwitches(QStringLiteral("KBoardGamepad"));
            QVERIFY(before >= 0);
            QTest::qWait(1000);
            const qint64 idle = contextSwitches(QStringLiteral("KBoardGamepad")) - before;
            QVERIFY2(idle <= 2, qPrintable(QStringLiteral("backend thread woke %1 times in one idle second").arg(idle)));
        }

        KBoardSettings::self()->setControllerEnabled(false);
        QVERIFY(!gamepad.available());
        QVERIFY(!gamepad.connected());

        KBoardSettings::self()->setControllerEnabled(true);
        QTRY_VERIFY_WITH_TIMEOUT(gamepad.available(), 5000);
    }
};

QTEST_GUILESS_MAIN(GamepadBackendTest)
#include "gamepadbackendtest.moc"
