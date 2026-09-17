#include "steamdedupe.h"

#include <QTest>

class SteamDedupeTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void detection()
    {
        QVERIFY(SteamDedupe::isSteamVirtual(0x28de, 0x11ff, 0, QStringLiteral("Xbox 360 Controller")));
        QVERIFY(SteamDedupe::isSteamVirtual(0x045e, 0x028e, 42, QStringLiteral("Xbox 360 Controller")));
        QVERIFY(SteamDedupe::isSteamVirtual(0, 0, 0, QStringLiteral("Steam Virtual Gamepad 2")));
        QVERIFY(!SteamDedupe::isSteamVirtual(0x28de, 0x1205, 0, QStringLiteral("Steam Deck")));
        QVERIFY(!SteamDedupe::isSteamVirtual(0x054c, 0x0ce6, 0, QStringLiteral("DualSense Wireless Controller")));
    }

    void noVirtualNoIgnore()
    {
        SteamDedupe dedupe;
        dedupe.addDevice({1, 0x054c, 0x0ce6, false});
        dedupe.addDevice({2, 0x045e, 0x0b12, false});
        QVERIFY(!dedupe.isIgnored(1));
        QVERIFY(!dedupe.isIgnored(2));
        QVERIFY(dedupe.press(1, QStringLiteral("a"), 100).forward);
        QVERIFY(dedupe.press(2, QStringLiteral("a"), 101).forward);
        QVERIFY(!dedupe.isIgnored(1));
        QVERIFY(!dedupe.isIgnored(2));
    }

    void identityPairing()
    {
        SteamDedupe dedupe;
        dedupe.addDevice({1, 0x054c, 0x0ce6, false});
        dedupe.addDevice({2, 0x045e, 0x0b12, false});
        dedupe.addDevice({3, 0x054c, 0x0ce6, true});
        QVERIFY(dedupe.isIgnored(1));
        QVERIFY(!dedupe.isIgnored(2));
        QVERIFY(!dedupe.isIgnored(3));
        QCOMPARE(dedupe.physicalFor(3), quint32(1));
        QCOMPARE(dedupe.virtualFor(1), quint32(3));
        QVERIFY(!dedupe.reasonIgnored(1).isEmpty());
        QVERIFY(!dedupe.press(1, QStringLiteral("a"), 10).forward);
        QVERIFY(dedupe.press(3, QStringLiteral("a"), 11).forward);

        dedupe.removeDevice(3);
        QVERIFY(!dedupe.isIgnored(1));
    }

    void identityPairingPhysicalArrivesLater()
    {
        SteamDedupe dedupe;
        dedupe.addDevice({5, 0x057e, 0x2009, true});
        QVERIFY(!dedupe.isIgnored(5));
        dedupe.addDevice({6, 0x057e, 0x2009, false});
        QVERIFY(dedupe.isIgnored(6));
    }

    void twoIdenticalPadsPairOneToOne()
    {
        SteamDedupe dedupe;
        dedupe.addDevice({1, 0x045e, 0x0b12, false});
        dedupe.addDevice({2, 0x045e, 0x0b12, false});
        dedupe.addDevice({3, 0x045e, 0x0b12, true});
        QVERIFY(dedupe.isIgnored(1) != dedupe.isIgnored(2));
        dedupe.addDevice({4, 0x045e, 0x0b12, true});
        QVERIFY(dedupe.isIgnored(1));
        QVERIFY(dedupe.isIgnored(2));
        QVERIFY(dedupe.physicalFor(3) != dedupe.physicalFor(4));
    }

    void correlationPhysicalFirst()
    {
        SteamDedupe dedupe;
        dedupe.addDevice({1, 0x045e, 0x028e, false});
        dedupe.addDevice({2, 0x054c, 0x0ce6, false});
        dedupe.addDevice({3, SteamDedupe::ValveVendor, SteamDedupe::SteamVirtualProduct, true});
        QVERIFY(!dedupe.isIgnored(1));
        QVERIFY(!dedupe.isIgnored(2));
        QVERIFY(dedupe.hasUnpairedVirtual());

        const auto physical = dedupe.press(2, QStringLiteral("x"), 1000);
        QVERIFY(physical.forward);
        const auto virtualPress = dedupe.press(3, QStringLiteral("x"), 1004);
        QVERIFY(!virtualPress.forward);
        QCOMPARE(virtualPress.replacedPhysical, quint32(2));
        QVERIFY(dedupe.isIgnored(2));
        QVERIFY(!dedupe.isIgnored(1));
        QVERIFY(!dedupe.hasUnpairedVirtual());
        QVERIFY(dedupe.press(3, QStringLiteral("a"), 2000).forward);
        QVERIFY(!dedupe.press(2, QStringLiteral("a"), 2001).forward);
        QVERIFY(dedupe.press(1, QStringLiteral("a"), 2002).forward);
    }

    void correlationVirtualFirst()
    {
        SteamDedupe dedupe;
        dedupe.addDevice({1, 0x045e, 0x028e, false});
        dedupe.addDevice({3, SteamDedupe::ValveVendor, SteamDedupe::SteamVirtualProduct, true});
        QVERIFY(dedupe.press(3, QStringLiteral("b"), 500).forward);
        const auto physical = dedupe.press(1, QStringLiteral("b"), 503);
        QVERIFY(!physical.forward);
        QCOMPARE(physical.replacedPhysical, quint32(0));
        QVERIFY(dedupe.isIgnored(1));
        QCOMPARE(dedupe.physicalFor(3), quint32(1));
    }

    void correlationNeedsSameButtonInWindow()
    {
        SteamDedupe dedupe;
        dedupe.addDevice({1, 0x045e, 0x028e, false});
        dedupe.addDevice({3, SteamDedupe::ValveVendor, SteamDedupe::SteamVirtualProduct, true});
        QVERIFY(dedupe.press(1, QStringLiteral("a"), 0).forward);
        QVERIFY(dedupe.press(3, QStringLiteral("b"), 5).forward);
        QVERIFY(dedupe.press(1, QStringLiteral("y"), 1000).forward);
        QVERIFY(dedupe.press(3, QStringLiteral("y"), 1000 + SteamDedupe::CorrelationWindowMs + 1).forward);
        QVERIFY(!dedupe.isIgnored(1));
        QVERIFY(!dedupe.press(1, QStringLiteral("y"), 1000 + SteamDedupe::CorrelationWindowMs + 20).forward);
        QVERIFY(dedupe.isIgnored(1));
    }

    void virtualsNeverPairWithVirtuals()
    {
        SteamDedupe dedupe;
        dedupe.addDevice({3, SteamDedupe::ValveVendor, SteamDedupe::SteamVirtualProduct, true});
        dedupe.addDevice({4, SteamDedupe::ValveVendor, SteamDedupe::SteamVirtualProduct, true});
        QVERIFY(dedupe.press(3, QStringLiteral("a"), 0).forward);
        QVERIFY(dedupe.press(4, QStringLiteral("a"), 1).forward);
        QVERIFY(!dedupe.isIgnored(3));
        QVERIFY(!dedupe.isIgnored(4));
    }

    void removingPhysicalUnpairs()
    {
        SteamDedupe dedupe;
        dedupe.addDevice({1, 0x045e, 0x028e, false});
        dedupe.addDevice({3, SteamDedupe::ValveVendor, SteamDedupe::SteamVirtualProduct, true});
        dedupe.press(1, QStringLiteral("a"), 0);
        dedupe.press(3, QStringLiteral("a"), 2);
        QVERIFY(dedupe.isIgnored(1));
        dedupe.removeDevice(1);
        QCOMPARE(dedupe.physicalFor(3), quint32(0));
        QVERIFY(dedupe.hasUnpairedVirtual());
    }
};

QTEST_GUILESS_MAIN(SteamDedupeTest)
#include "steamdedupetest.moc"
