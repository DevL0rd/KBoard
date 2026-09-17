#include "descriptorscanner.h"

#include <QFile>
#include <QTemporaryDir>
#include <QTest>

class DescriptorScannerTest : public QObject
{
    Q_OBJECT

private:
    static void link(const QTemporaryDir &dir, int descriptor, const QString &target)
    {
        QVERIFY(QFile::link(target, dir.filePath(QString::number(descriptor))));
    }

private Q_SLOTS:
    void netlinkTable()
    {
        QTemporaryDir dir;
        QFile table(dir.filePath(QStringLiteral("netlink")));
        QVERIFY(table.open(QIODevice::WriteOnly));
        table.write("sk               Eth Pid        Groups   Rmem     Wmem     Dump  Locks    Drops    Inode\n");
        table.write("0000000000000000 15  0          00000002 0        0        0     2        0        111\n");
        table.write("0000000000000000 0   3117       00000440 0        0        0     2        0        222\n");
        table.write("0000000000000000 15  4242       00000001 0        0        0     2        0        333\n");
        table.close();
        QCOMPARE(DescriptorScanner::ueventSocketInodes(table.fileName()), (QSet<quint64> {111, 333}));
        QVERIFY(DescriptorScanner::ueventSocketInodes(dir.filePath(QStringLiteral("missing"))).isEmpty());
    }

    void targets()
    {
        const QSet<quint64> uevents {111};
        QVERIFY(DescriptorScanner::isInputTarget(QStringLiteral("/dev/input/event7"), uevents));
        QVERIFY(DescriptorScanner::isInputTarget(QStringLiteral("/dev/hidraw3"), uevents));
        QVERIFY(DescriptorScanner::isInputTarget(QStringLiteral("/dev/bus/usb/001/004"), uevents));
        QVERIFY(DescriptorScanner::isInputTarget(QStringLiteral("socket:[111]"), uevents));
        QVERIFY(!DescriptorScanner::isInputTarget(QStringLiteral("socket:[222]"), uevents));
        QVERIFY(!DescriptorScanner::isInputTarget(QStringLiteral("/dev/input/mice"), uevents));
        QVERIFY(!DescriptorScanner::isInputTarget(QStringLiteral("anon_inode:[eventfd]"), uevents));
        QVERIFY(!DescriptorScanner::isInputTarget(QStringLiteral("/home/user/socket:[111]"), uevents));
    }

    void newDescriptorsSinceBaseline()
    {
        QTemporaryDir dir;
        link(dir, 3, QStringLiteral("/dev/hidraw1"));
        link(dir, 4, QStringLiteral("socket:[111]"));
        link(dir, 5, QStringLiteral("/dev/input/event2"));
        const DescriptorScanner::Snapshot baseline = DescriptorScanner::snapshot(dir.path());
        QCOMPARE(baseline.value(4), QStringLiteral("socket:[111]"));

        QVERIFY(QFile::remove(dir.filePath(QStringLiteral("5"))));
        link(dir, 5, QStringLiteral("/dev/input/event9"));
        link(dir, 6, QStringLiteral("socket:[112]"));
        link(dir, 7, QStringLiteral("/dev/hidraw4"));
        link(dir, 8, QStringLiteral("/tmp/file"));
        link(dir, 9, QStringLiteral("socket:[999]"));

        const QList<int> descriptors = DescriptorScanner::inputDescriptors(baseline, DescriptorScanner::snapshot(dir.path()), {111, 112});
        QCOMPARE(descriptors, (QList<int> {5, 6, 7}));
    }
};

QTEST_GUILESS_MAIN(DescriptorScannerTest)
#include "descriptorscannertest.moc"
