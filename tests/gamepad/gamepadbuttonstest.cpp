#include "gamepadbuttons.h"

#include <QTest>

#include <SDL3/SDL_gamepad.h>

class GamepadButtonsTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void sdlNames()
    {
        QCOMPARE(GamepadButtons::nameForSdlButton(SDL_GAMEPAD_BUTTON_SOUTH), QStringLiteral("a"));
        QCOMPARE(GamepadButtons::nameForSdlButton(SDL_GAMEPAD_BUTTON_EAST), QStringLiteral("b"));
        QCOMPARE(GamepadButtons::nameForSdlButton(SDL_GAMEPAD_BUTTON_WEST), QStringLiteral("x"));
        QCOMPARE(GamepadButtons::nameForSdlButton(SDL_GAMEPAD_BUTTON_NORTH), QStringLiteral("y"));
        QCOMPARE(GamepadButtons::nameForSdlButton(SDL_GAMEPAD_BUTTON_BACK), QStringLiteral("back"));
        QCOMPARE(GamepadButtons::nameForSdlButton(SDL_GAMEPAD_BUTTON_START), QStringLiteral("start"));
        QCOMPARE(GamepadButtons::nameForSdlButton(SDL_GAMEPAD_BUTTON_GUIDE), QStringLiteral("guide"));
        QCOMPARE(GamepadButtons::nameForSdlButton(SDL_GAMEPAD_BUTTON_LEFT_SHOULDER), QStringLiteral("leftshoulder"));
        QCOMPARE(GamepadButtons::nameForSdlButton(SDL_GAMEPAD_BUTTON_RIGHT_STICK), QStringLiteral("rightstick"));
        QCOMPARE(GamepadButtons::nameForSdlButton(SDL_GAMEPAD_BUTTON_DPAD_UP), QStringLiteral("dpup"));
        QCOMPARE(GamepadButtons::nameForSdlButton(SDL_GAMEPAD_BUTTON_DPAD_RIGHT), QStringLiteral("dpright"));
        QCOMPARE(GamepadButtons::nameForSdlButton(SDL_GAMEPAD_BUTTON_MISC6), QStringLiteral("misc6"));
        QCOMPARE(GamepadButtons::nameForSdlButton(SDL_GAMEPAD_BUTTON_COUNT), QString());
        QCOMPARE(GamepadButtons::nameForSdlButton(-1), QString());
        QCOMPARE(GamepadButtons::sdlButtonForName(QStringLiteral("dpleft")), int(SDL_GAMEPAD_BUTTON_DPAD_LEFT));
    }

    void contractNamesKnown()
    {
        const QStringList contract
            = {QStringLiteral("a"), QStringLiteral("b"), QStringLiteral("x"), QStringLiteral("y"), QStringLiteral("leftshoulder"),
                QStringLiteral("rightshoulder"), QStringLiteral("lefttrigger"), QStringLiteral("righttrigger"), QStringLiteral("back"),
                QStringLiteral("start"), QStringLiteral("guide"), QStringLiteral("leftstick"), QStringLiteral("rightstick"),
                QStringLiteral("dpup"), QStringLiteral("dpdown"), QStringLiteral("dpleft"), QStringLiteral("dpright")};
        for (const QString &name : contract) {
            QVERIFY2(GamepadButtons::names().contains(name), qPrintable(name));
            for (const QString &type : GamepadButtons::controllerTypes()) {
                QVERIFY2(!GamepadButtons::glyph(type, name).isEmpty(), qPrintable(type + QLatin1Char(':') + name));
            }
        }
    }

    void aliases_data()
    {
        QTest::addColumn<QString>("input");
        QTest::addColumn<QString>("canonical");
        QTest::newRow("select") << QStringLiteral("Select") << QStringLiteral("back");
        QTest::newRow("view") << QStringLiteral("view") << QStringLiteral("back");
        QTest::newRow("lb") << QStringLiteral("LB") << QStringLiteral("leftshoulder");
        QTest::newRow("zr") << QStringLiteral("zr") << QStringLiteral("righttrigger");
        QTest::newRow("up") << QStringLiteral(" up ") << QStringLiteral("dpup");
        QTest::newRow("options") << QStringLiteral("options") << QStringLiteral("start");
        QTest::newRow("canonical") << QStringLiteral("x") << QStringLiteral("x");
        QTest::newRow("unknown") << QStringLiteral("banana") << QString();
    }

    void aliases()
    {
        QFETCH(QString, input);
        QFETCH(QString, canonical);
        QCOMPARE(GamepadButtons::canonicalName(input), canonical);
    }

    void glyphs_data()
    {
        QTest::addColumn<QString>("type");
        QTest::addColumn<QString>("button");
        QTest::addColumn<QString>("glyph");
        QTest::newRow("xbox a") << QStringLiteral("xbox") << QStringLiteral("a") << QStringLiteral("A");
        QTest::newRow("xbox b") << QStringLiteral("xbox") << QStringLiteral("b") << QStringLiteral("B");
        QTest::newRow("xbox x") << QStringLiteral("xbox") << QStringLiteral("x") << QStringLiteral("X");
        QTest::newRow("xbox y") << QStringLiteral("xbox") << QStringLiteral("y") << QStringLiteral("Y");
        QTest::newRow("xbox lb") << QStringLiteral("xbox") << QStringLiteral("leftshoulder") << QStringLiteral("LB");
        QTest::newRow("xbox back") << QStringLiteral("xbox") << QStringLiteral("back") << QStringLiteral("View");
        QTest::newRow("ps a") << QStringLiteral("playstation") << QStringLiteral("a") << QStringLiteral("✕");
        QTest::newRow("ps b") << QStringLiteral("playstation") << QStringLiteral("b") << QStringLiteral("○");
        QTest::newRow("ps x") << QStringLiteral("playstation") << QStringLiteral("x") << QStringLiteral("□");
        QTest::newRow("ps y") << QStringLiteral("playstation") << QStringLiteral("y") << QStringLiteral("△");
        QTest::newRow("ps lt") << QStringLiteral("playstation") << QStringLiteral("lefttrigger") << QStringLiteral("L2");
        QTest::newRow("ps start") << QStringLiteral("playstation") << QStringLiteral("start") << QStringLiteral("Options");
        QTest::newRow("nintendo south") << QStringLiteral("nintendo") << QStringLiteral("a") << QStringLiteral("B");
        QTest::newRow("nintendo east") << QStringLiteral("nintendo") << QStringLiteral("b") << QStringLiteral("A");
        QTest::newRow("nintendo west") << QStringLiteral("nintendo") << QStringLiteral("x") << QStringLiteral("Y");
        QTest::newRow("nintendo north") << QStringLiteral("nintendo") << QStringLiteral("y") << QStringLiteral("X");
        QTest::newRow("nintendo zr") << QStringLiteral("nintendo") << QStringLiteral("righttrigger") << QStringLiteral("ZR");
        QTest::newRow("nintendo back") << QStringLiteral("nintendo") << QStringLiteral("back") << QStringLiteral("−");
        QTest::newRow("deck a") << QStringLiteral("steamdeck") << QStringLiteral("a") << QStringLiteral("A");
        QTest::newRow("deck y") << QStringLiteral("steamdeck") << QStringLiteral("y") << QStringLiteral("Y");
        QTest::newRow("deck guide") << QStringLiteral("steamdeck") << QStringLiteral("guide") << QStringLiteral("Steam");
        QTest::newRow("deck l4") << QStringLiteral("steamdeck") << QStringLiteral("leftpaddle1") << QStringLiteral("L4");
        QTest::newRow("generic start") << QStringLiteral("generic") << QStringLiteral("start") << QStringLiteral("Start");
        QTest::newRow("alias glyph") << QStringLiteral("playstation") << QStringLiteral("select") << QStringLiteral("Create");
        QTest::newRow("dpad") << QStringLiteral("xbox") << QStringLiteral("dpleft") << QStringLiteral("←");
        QTest::newRow("unknown button") << QStringLiteral("xbox") << QStringLiteral("banana") << QString();
    }

    void glyphs()
    {
        QFETCH(QString, type);
        QFETCH(QString, button);
        QFETCH(QString, glyph);
        QCOMPARE(GamepadButtons::glyph(type, button), glyph);
    }

    void types()
    {
        QCOMPARE(GamepadButtons::typeForSdl(SDL_GAMEPAD_TYPE_XBOXONE, 0x045e, 0x0b12), QStringLiteral("xbox"));
        QCOMPARE(GamepadButtons::typeForSdl(SDL_GAMEPAD_TYPE_XBOX360, 0x045e, 0x028e), QStringLiteral("xbox"));
        QCOMPARE(GamepadButtons::typeForSdl(SDL_GAMEPAD_TYPE_PS5, 0x054c, 0x0ce6), QStringLiteral("playstation"));
        QCOMPARE(GamepadButtons::typeForSdl(SDL_GAMEPAD_TYPE_PS4, 0x054c, 0x09cc), QStringLiteral("playstation"));
        QCOMPARE(GamepadButtons::typeForSdl(SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_PRO, 0x057e, 0x2009), QStringLiteral("nintendo"));
        QCOMPARE(GamepadButtons::typeForSdl(SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_PAIR, 0x057e, 0x2008), QStringLiteral("nintendo"));
        QCOMPARE(GamepadButtons::typeForSdl(SDL_GAMEPAD_TYPE_STANDARD, 0x28de, 0x1205), QStringLiteral("steamdeck"));
        QCOMPARE(GamepadButtons::typeForSdl(SDL_GAMEPAD_TYPE_XBOX360, 0x28de, 0x11ff), QStringLiteral("xbox"));
        QCOMPARE(GamepadButtons::typeForSdl(SDL_GAMEPAD_TYPE_STANDARD, 0x28de, 0x11ff), QStringLiteral("xbox"));
        QCOMPARE(GamepadButtons::typeForSdl(SDL_GAMEPAD_TYPE_STANDARD, 0x1234, 0x5678), QStringLiteral("generic"));
        QCOMPARE(GamepadButtons::typeForSdl(SDL_GAMEPAD_TYPE_UNKNOWN, 0, 0), QStringLiteral("generic"));
    }
};

QTEST_GUILESS_MAIN(GamepadButtonsTest)
#include "gamepadbuttonstest.moc"
