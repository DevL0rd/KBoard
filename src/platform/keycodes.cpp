#include "keycodes.h"
#include "inputcontext.h"

#include <QHash>

#include <linux/input-event-codes.h>

namespace KeyCodes
{
const QHash<QString, uint> names = {
    {QStringLiteral("a"), KEY_A},
    {QStringLiteral("b"), KEY_B},
    {QStringLiteral("c"), KEY_C},
    {QStringLiteral("d"), KEY_D},
    {QStringLiteral("e"), KEY_E},
    {QStringLiteral("f"), KEY_F},
    {QStringLiteral("g"), KEY_G},
    {QStringLiteral("h"), KEY_H},
    {QStringLiteral("i"), KEY_I},
    {QStringLiteral("j"), KEY_J},
    {QStringLiteral("k"), KEY_K},
    {QStringLiteral("l"), KEY_L},
    {QStringLiteral("m"), KEY_M},
    {QStringLiteral("n"), KEY_N},
    {QStringLiteral("o"), KEY_O},
    {QStringLiteral("p"), KEY_P},
    {QStringLiteral("q"), KEY_Q},
    {QStringLiteral("r"), KEY_R},
    {QStringLiteral("s"), KEY_S},
    {QStringLiteral("t"), KEY_T},
    {QStringLiteral("u"), KEY_U},
    {QStringLiteral("v"), KEY_V},
    {QStringLiteral("w"), KEY_W},
    {QStringLiteral("x"), KEY_X},
    {QStringLiteral("y"), KEY_Y},
    {QStringLiteral("z"), KEY_Z},
    {QStringLiteral("0"), KEY_0},
    {QStringLiteral("1"), KEY_1},
    {QStringLiteral("2"), KEY_2},
    {QStringLiteral("3"), KEY_3},
    {QStringLiteral("4"), KEY_4},
    {QStringLiteral("5"), KEY_5},
    {QStringLiteral("6"), KEY_6},
    {QStringLiteral("7"), KEY_7},
    {QStringLiteral("8"), KEY_8},
    {QStringLiteral("9"), KEY_9},
    {QStringLiteral("esc"), KEY_ESC},
    {QStringLiteral("escape"), KEY_ESC},
    {QStringLiteral("tab"), KEY_TAB},
    {QStringLiteral("enter"), KEY_ENTER},
    {QStringLiteral("return"), KEY_ENTER},
    {QStringLiteral("space"), KEY_SPACE},
    {QStringLiteral("backspace"), KEY_BACKSPACE},
    {QStringLiteral("delete"), KEY_DELETE},
    {QStringLiteral("insert"), KEY_INSERT},
    {QStringLiteral("home"), KEY_HOME},
    {QStringLiteral("end"), KEY_END},
    {QStringLiteral("pageup"), KEY_PAGEUP},
    {QStringLiteral("pagedown"), KEY_PAGEDOWN},
    {QStringLiteral("left"), KEY_LEFT},
    {QStringLiteral("right"), KEY_RIGHT},
    {QStringLiteral("up"), KEY_UP},
    {QStringLiteral("down"), KEY_DOWN},
    {QStringLiteral("f1"), KEY_F1},
    {QStringLiteral("f2"), KEY_F2},
    {QStringLiteral("f3"), KEY_F3},
    {QStringLiteral("f4"), KEY_F4},
    {QStringLiteral("f5"), KEY_F5},
    {QStringLiteral("f6"), KEY_F6},
    {QStringLiteral("f7"), KEY_F7},
    {QStringLiteral("f8"), KEY_F8},
    {QStringLiteral("f9"), KEY_F9},
    {QStringLiteral("f10"), KEY_F10},
    {QStringLiteral("f11"), KEY_F11},
    {QStringLiteral("f12"), KEY_F12},
    {QStringLiteral("minus"), KEY_MINUS},
    {QStringLiteral("equal"), KEY_EQUAL},
    {QStringLiteral("slash"), KEY_SLASH},
    {QStringLiteral("print"), KEY_SYSRQ},
};

uint evdevForName(const QString &name)
{
    return names.value(name, 0);
}

uint modifierForName(const QString &name)
{
    if (name == QLatin1String("ctrl") || name == QLatin1String("control")) {
        return InputContext::ControlModifier;
    }
    if (name == QLatin1String("shift")) {
        return InputContext::ShiftModifier;
    }
    if (name == QLatin1String("alt")) {
        return InputContext::AltModifier;
    }
    if (name == QLatin1String("super") || name == QLatin1String("meta")) {
        return InputContext::SuperModifier;
    }
    return 0;
}
}
