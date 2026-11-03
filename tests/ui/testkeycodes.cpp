#include "inputcontext.h"
#include "keycodes.h"
#include "keynames.h"

#include <QTest>

#include <linux/input-event-codes.h>

using KeyCodes::KeyEvent;

class TestKeyCodes : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void controlWrapsTheKey()
    {
        const QList<KeyEvent> expected {{KEY_LEFTCTRL, true}, {KEY_C, true}, {KEY_C, false}, {KEY_LEFTCTRL, false}};
        QCOMPARE(KeyCodes::chord(InputContext::ControlModifier, KEY_C), expected);
    }

    void metaAloneIsPressedAndReleased()
    {
        const QList<KeyEvent> expected {{KEY_LEFTMETA, true}, {KEY_LEFTMETA, false}};
        QCOMPARE(KeyCodes::chord(InputContext::SuperModifier, 0), expected);
    }

    void modifiersReleaseInReverseOrder()
    {
        const uint all = InputContext::ControlModifier | InputContext::ShiftModifier | InputContext::AltModifier | InputContext::SuperModifier;
        const QList<KeyEvent> expected {{KEY_LEFTCTRL, true}, {KEY_LEFTSHIFT, true}, {KEY_LEFTALT, true}, {KEY_LEFTMETA, true}, {KEY_Z, true},
            {KEY_Z, false}, {KEY_LEFTMETA, false}, {KEY_LEFTALT, false}, {KEY_LEFTSHIFT, false}, {KEY_LEFTCTRL, false}};
        QCOMPARE(KeyCodes::chord(all, KEY_Z), expected);
    }

    void plainKeyHasNoModifierEvents()
    {
        const QList<KeyEvent> expected {{KEY_C, true}, {KEY_C, false}};
        QCOMPARE(KeyCodes::chord(0, KEY_C), expected);
    }

    void capsLockBitSendsNoKey()
    {
        const QList<KeyEvent> expected {{KEY_C, true}, {KEY_C, false}};
        QCOMPARE(KeyCodes::chord(InputContext::LockModifier, KEY_C), expected);
    }

    void everyModifierEveryEventIsBalanced()
    {
        for (const char *name : {"ctrl", "alt", "super", "shift"}) {
            const auto events = KeyCodes::chord(KeyCodes::modifierForName(QString::fromLatin1(name)), KEY_A);
            int down = 0;
            for (const KeyEvent &event : events) {
                down += event.pressed ? 1 : -1;
                QVERIFY(down >= 0);
            }
            QCOMPARE(down, 0);
        }
    }

    void lettersNeverNeedShiftForShortcuts()
    {
        KeyNames names;
        QVERIFY(!names.symbolNeedsShift(QStringLiteral("C")));
        QVERIFY(!names.symbolNeedsShift(QStringLiteral("c")));
        QVERIFY(names.symbolNeedsShift(QStringLiteral("?")));
        QVERIFY(!names.symbolNeedsShift(QStringLiteral("/")));
    }
};

QTEST_GUILESS_MAIN(TestKeyCodes)
#include "testkeycodes.moc"
