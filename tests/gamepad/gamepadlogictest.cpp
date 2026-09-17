#include "gamepad.h"
#include "kboardsettings.h"

#include <QSignalSpy>
#include <QStandardPaths>
#include <QTest>

#include <SDL3/SDL_gamepad.h>

namespace
{
GamepadDevice device(quint32 id, const QString &type, quint16 vendor, quint16 product, bool steamVirtual = false)
{
    GamepadDevice info;
    info.id = id;
    info.name = QStringLiteral("Pad %1").arg(id);
    info.controllerType = type;
    info.vendor = vendor;
    info.product = product;
    info.rawVendor = steamVirtual ? 0x28de : vendor;
    info.rawProduct = steamVirtual ? 0x11ff : product;
    info.steamVirtual = steamVirtual;
    return info;
}

QStringList names(const QSignalSpy &spy)
{
    QStringList result;
    for (const auto &arguments : spy) {
        result.append(arguments.at(0).toString());
    }
    return result;
}
}

class GamepadLogicTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);
        KBoardSettings::self()->setDefaults();
    }

    void init() { KBoardSettings::self()->setDefaults(); }

    void hotplugAndActivePad()
    {
        Gamepad gamepad(Gamepad::Mode::Detached);
        QSignalSpy active(&gamepad, &Gamepad::activeDeviceChanged);
        QVERIFY(!gamepad.connected());
        QCOMPARE(gamepad.controllerType(), QStringLiteral("generic"));

        gamepad.handleDeviceAdded(device(1, QStringLiteral("xbox"), 0x045e, 0x0b12));
        QVERIFY(gamepad.connected());
        QCOMPARE(gamepad.activeDeviceId(), 1);
        QCOMPARE(gamepad.glyph(QStringLiteral("a")), QStringLiteral("A"));

        gamepad.handleDeviceAdded(device(2, QStringLiteral("playstation"), 0x054c, 0x0ce6));
        QCOMPARE(gamepad.activeDeviceId(), 1);
        QCOMPARE(gamepad.devices().size(), 2);

        QSignalSpy pressed(&gamepad, &Gamepad::buttonPressed);
        gamepad.handleButton(2, QStringLiteral("a"), true, 10);
        QCOMPARE(gamepad.activeDeviceId(), 2);
        QCOMPARE(gamepad.controllerType(), QStringLiteral("playstation"));
        QCOMPARE(gamepad.glyph(QStringLiteral("a")), QStringLiteral("✕"));
        QCOMPARE(names(pressed), QStringList {QStringLiteral("a")});

        gamepad.handleAxis(1, SDL_GAMEPAD_AXIS_RIGHTX, 0.1, 20);
        QCOMPARE(gamepad.activeDeviceId(), 2);
        gamepad.handleAxis(1, SDL_GAMEPAD_AXIS_RIGHTX, 0.9, 30);
        QCOMPARE(gamepad.activeDeviceId(), 1);
        QCOMPARE(gamepad.rightStick(), QPointF(0.9, 0.0));

        gamepad.handleDeviceRemoved(1);
        QCOMPARE(gamepad.activeDeviceId(), 2);
        QVERIFY(gamepad.rightStick().isNull());
        gamepad.handleDeviceRemoved(2);
        QVERIFY(!gamepad.connected());
        QVERIFY(active.count() >= 4);
    }

    void switchingPadReleasesHeldButtons()
    {
        Gamepad gamepad(Gamepad::Mode::Detached);
        gamepad.handleDeviceAdded(device(1, QStringLiteral("xbox"), 0x045e, 0x0b12));
        gamepad.handleDeviceAdded(device(2, QStringLiteral("xbox"), 0x045e, 0x0b13));
        QSignalSpy released(&gamepad, &Gamepad::buttonReleased);
        gamepad.handleButton(1, QStringLiteral("y"), true, 0);
        QCOMPARE(gamepad.heldButtons(), QStringList {QStringLiteral("y")});
        gamepad.handleButton(2, QStringLiteral("b"), true, 5);
        QCOMPARE(names(released), QStringList {QStringLiteral("y")});
        QCOMPARE(gamepad.heldButtons(), QStringList {QStringLiteral("b")});
        gamepad.handleButton(1, QStringLiteral("y"), false, 6);
        QCOMPARE(released.count(), 1);
    }

    void chordSuppressesCompletingPress()
    {
        Gamepad gamepad(Gamepad::Mode::Detached);
        gamepad.handleDeviceAdded(device(1, QStringLiteral("xbox"), 0x045e, 0x0b12));
        QCOMPARE(gamepad.chord(), (QStringList {QStringLiteral("back"), QStringLiteral("x")}));
        QSignalSpy pressed(&gamepad, &Gamepad::buttonPressed);
        QSignalSpy released(&gamepad, &Gamepad::buttonReleased);
        QSignalSpy tapped(&gamepad, &Gamepad::buttonTapped);
        QSignalSpy chord(&gamepad, &Gamepad::chordActivated);

        gamepad.handleButton(1, QStringLiteral("back"), true, 0);
        gamepad.handleButton(1, QStringLiteral("x"), true, 50);
        QCOMPARE(chord.count(), 1);
        QCOMPARE(names(pressed), QStringList {QStringLiteral("back")});
        gamepad.handleButton(1, QStringLiteral("x"), false, 100);
        gamepad.handleButton(1, QStringLiteral("back"), false, 120);
        QCOMPARE(names(released), QStringList {QStringLiteral("back")});
        QVERIFY(tapped.isEmpty());

        gamepad.handleButton(1, QStringLiteral("back"), true, 200);
        gamepad.handleButton(1, QStringLiteral("back"), false, 250);
        QCOMPARE(names(tapped), QStringList {QStringLiteral("back")});
        QCOMPARE(chord.count(), 1);
    }

    void chordFiresOncePerHold()
    {
        Gamepad gamepad(Gamepad::Mode::Detached);
        gamepad.handleDeviceAdded(device(1, QStringLiteral("xbox"), 0x045e, 0x0b12));
        QSignalSpy chord(&gamepad, &Gamepad::chordActivated);
        gamepad.handleButton(1, QStringLiteral("back"), true, 0);
        gamepad.handleButton(1, QStringLiteral("x"), true, 1);
        gamepad.handleButton(1, QStringLiteral("x"), true, 2);
        gamepad.handleButton(1, QStringLiteral("a"), true, 3);
        QCOMPARE(chord.count(), 1);
        gamepad.handleButton(1, QStringLiteral("x"), false, 4);
        gamepad.handleButton(1, QStringLiteral("x"), true, 5);
        QCOMPARE(chord.count(), 2);
    }

    void chordFromSettings()
    {
        Gamepad gamepad(Gamepad::Mode::Detached);
        QSignalSpy changed(&gamepad, &Gamepad::chordChanged);
        KBoardSettings::self()->setControllerOpenChord(QStringLiteral("lb+rb+start"));
        QCOMPARE(changed.count(), 1);
        QCOMPARE(gamepad.chord(), (QStringList {QStringLiteral("leftshoulder"), QStringLiteral("rightshoulder"), QStringLiteral("start")}));
        QVERIFY(gamepad.chordError().isEmpty());
        KBoardSettings::self()->setControllerOpenChord(QStringLiteral("lb+nope"));
        QVERIFY(gamepad.chord().isEmpty());
        QVERIFY(gamepad.chordError().contains(QStringLiteral("nope")));
    }

    void triggersBecomeButtons()
    {
        Gamepad gamepad(Gamepad::Mode::Detached);
        gamepad.handleDeviceAdded(device(1, QStringLiteral("xbox"), 0x045e, 0x0b12));
        QSignalSpy pressed(&gamepad, &Gamepad::buttonPressed);
        QSignalSpy released(&gamepad, &Gamepad::buttonReleased);
        gamepad.handleAxis(1, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER, 0.4, 0);
        QVERIFY(pressed.isEmpty());
        QCOMPARE(gamepad.rightTrigger(), 0.4);
        gamepad.handleAxis(1, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER, 0.8, 1);
        QCOMPARE(names(pressed), QStringList {QStringLiteral("righttrigger")});
        gamepad.handleAxis(1, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER, 0.45, 2);
        QVERIFY(released.isEmpty());
        gamepad.handleAxis(1, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER, 0.1, 3);
        QCOMPARE(names(released), QStringList {QStringLiteral("righttrigger")});
    }

    void dpadAndStickNavigate()
    {
        Gamepad gamepad(Gamepad::Mode::Detached);
        gamepad.handleDeviceAdded(device(1, QStringLiteral("xbox"), 0x045e, 0x0b12));
        QSignalSpy navigate(&gamepad, &Gamepad::navigate);
        gamepad.handleButton(1, QStringLiteral("dpdown"), true, 0);
        QCOMPARE(navigate.count(), 1);
        QCOMPARE(navigate.at(0).at(1).toInt(), 1);
        gamepad.handleButton(1, QStringLiteral("dpdown"), false, 1);
        QTest::qWait(RepeatSchedule::InitialDelayMs + 50);
        QCOMPARE(navigate.count(), 1);

        gamepad.handleAxis(1, SDL_GAMEPAD_AXIS_LEFTX, -0.2, 2);
        QCOMPARE(navigate.count(), 1);
        gamepad.handleAxis(1, SDL_GAMEPAD_AXIS_LEFTX, -0.9, 3);
        QCOMPARE(navigate.count(), 2);
        QCOMPARE(navigate.at(1).at(0).toInt(), -1);
        QTRY_VERIFY_WITH_TIMEOUT(navigate.count() >= 3, 1000);
        gamepad.handleAxis(1, SDL_GAMEPAD_AXIS_LEFTX, 0.0, 4);
        const int count = navigate.count();
        QTest::qWait(200);
        QCOMPARE(navigate.count(), count);
    }

    void radialModeStopsStickNavigation()
    {
        Gamepad gamepad(Gamepad::Mode::Detached);
        gamepad.handleDeviceAdded(device(1, QStringLiteral("xbox"), 0x045e, 0x0b12));
        KBoardSettings::self()->setControllerRadialMode(true);
        QVERIFY(gamepad.radialMode());
        QSignalSpy navigate(&gamepad, &Gamepad::navigate);
        gamepad.handleAxis(1, SDL_GAMEPAD_AXIS_LEFTX, 1.0, 0);
        gamepad.handleAxis(1, SDL_GAMEPAD_AXIS_LEFTY, 0.0, 0);
        QVERIFY(navigate.isEmpty());
        QCOMPARE(gamepad.leftStickAngle(), 90.0);
        QCOMPARE(gamepad.leftStickMagnitude(), 1.0);
        gamepad.handleButton(1, QStringLiteral("dpright"), true, 1);
        QCOMPARE(navigate.count(), 1);
    }

    void stickAngles()
    {
        QCOMPARE(Gamepad::stickAngle(QPointF(0, -1)), 0.0);
        QCOMPARE(Gamepad::stickAngle(QPointF(1, 0)), 90.0);
        QCOMPARE(Gamepad::stickAngle(QPointF(0, 1)), 180.0);
        QCOMPARE(Gamepad::stickAngle(QPointF(-1, 0)), 270.0);
        QCOMPARE(Gamepad::stickMagnitude(QPointF(1, 1)), 1.0);
        QCOMPARE(Gamepad::stickMagnitude(QPointF(0.3, 0.4)), 0.5);
    }

    void steamVirtualTakesOverByIdentity()
    {
        Gamepad gamepad(Gamepad::Mode::Detached);
        gamepad.handleDeviceAdded(device(1, QStringLiteral("playstation"), 0x054c, 0x0ce6));
        QCOMPARE(gamepad.activeDeviceId(), 1);
        gamepad.handleDeviceAdded(device(7, QStringLiteral("playstation"), 0x054c, 0x0ce6, true));
        QCOMPARE(gamepad.activeDeviceId(), 7);
        QSignalSpy pressed(&gamepad, &Gamepad::buttonPressed);
        gamepad.handleButton(1, QStringLiteral("a"), true, 0);
        gamepad.handleButton(7, QStringLiteral("a"), true, 2);
        QCOMPARE(names(pressed), QStringList {QStringLiteral("a")});
        const QVariantList devices = gamepad.devices();
        QCOMPARE(devices.size(), 2);
        QVERIFY(devices.at(0).toMap().value(QStringLiteral("ignored")).toBool());
        QVERIFY(!devices.at(1).toMap().value(QStringLiteral("ignored")).toBool());
    }

    void steamVirtualTakesOverByCorrelation()
    {
        Gamepad gamepad(Gamepad::Mode::Detached);
        gamepad.handleDeviceAdded(device(1, QStringLiteral("xbox"), 0x045e, 0x028e));
        gamepad.handleDeviceAdded(device(9, QStringLiteral("xbox"), 0x28de, 0x11ff, true));
        QSignalSpy pressed(&gamepad, &Gamepad::buttonPressed);
        QSignalSpy released(&gamepad, &Gamepad::buttonReleased);
        gamepad.handleButton(1, QStringLiteral("a"), true, 1000);
        gamepad.handleButton(9, QStringLiteral("a"), true, 1003);
        QCOMPARE(names(pressed), QStringList {QStringLiteral("a")});
        QCOMPARE(gamepad.activeDeviceId(), 9);
        QCOMPARE(gamepad.heldButtons(), QStringList {QStringLiteral("a")});
        gamepad.handleButton(1, QStringLiteral("a"), false, 1050);
        QVERIFY(released.isEmpty());
        gamepad.handleButton(9, QStringLiteral("a"), false, 1052);
        QCOMPARE(names(released), QStringList {QStringLiteral("a")});
        gamepad.handleButton(1, QStringLiteral("b"), true, 2000);
        gamepad.handleButton(9, QStringLiteral("b"), true, 2002);
        QCOMPARE(names(pressed), (QStringList {QStringLiteral("a"), QStringLiteral("b")}));
    }

    void rumbleHonoursSetting()
    {
        Gamepad gamepad(Gamepad::Mode::Detached);
        GamepadDevice info = device(1, QStringLiteral("xbox"), 0x045e, 0x0b12);
        info.rumble = true;
        gamepad.handleDeviceAdded(info);
        QVERIFY(gamepad.rumbleSupported());
        KBoardSettings::self()->setControllerRumble(false);
        QVERIFY(!gamepad.rumble(0.5, 40));
    }
};

QTEST_GUILESS_MAIN(GamepadLogicTest)
#include "gamepadlogictest.moc"
