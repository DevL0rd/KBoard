#include "gamepadbuttons.h"

#include "steamdedupe.h"

#include <QHash>

#include <array>

#include <SDL3/SDL_gamepad.h>

namespace
{
const QStringList &sdlOrder()
{
    static const QStringList order = {
        QStringLiteral("a"),
        QStringLiteral("b"),
        QStringLiteral("x"),
        QStringLiteral("y"),
        QStringLiteral("back"),
        QStringLiteral("guide"),
        QStringLiteral("start"),
        QStringLiteral("leftstick"),
        QStringLiteral("rightstick"),
        QStringLiteral("leftshoulder"),
        QStringLiteral("rightshoulder"),
        QStringLiteral("dpup"),
        QStringLiteral("dpdown"),
        QStringLiteral("dpleft"),
        QStringLiteral("dpright"),
        QStringLiteral("misc1"),
        QStringLiteral("rightpaddle1"),
        QStringLiteral("leftpaddle1"),
        QStringLiteral("rightpaddle2"),
        QStringLiteral("leftpaddle2"),
        QStringLiteral("touchpad"),
        QStringLiteral("misc2"),
        QStringLiteral("misc3"),
        QStringLiteral("misc4"),
        QStringLiteral("misc5"),
        QStringLiteral("misc6"),
    };
    Q_ASSERT(order.size() == SDL_GAMEPAD_BUTTON_COUNT);
    return order;
}

const QHash<QString, QString> &aliases()
{
    static const QHash<QString, QString> table = {
        {QStringLiteral("select"), QStringLiteral("back")},
        {QStringLiteral("view"), QStringLiteral("back")},
        {QStringLiteral("share"), QStringLiteral("back")},
        {QStringLiteral("create"), QStringLiteral("back")},
        {QStringLiteral("minus"), QStringLiteral("back")},
        {QStringLiteral("menu"), QStringLiteral("start")},
        {QStringLiteral("options"), QStringLiteral("start")},
        {QStringLiteral("plus"), QStringLiteral("start")},
        {QStringLiteral("home"), QStringLiteral("guide")},
        {QStringLiteral("ps"), QStringLiteral("guide")},
        {QStringLiteral("steam"), QStringLiteral("guide")},
        {QStringLiteral("lb"), QStringLiteral("leftshoulder")},
        {QStringLiteral("rb"), QStringLiteral("rightshoulder")},
        {QStringLiteral("l1"), QStringLiteral("leftshoulder")},
        {QStringLiteral("r1"), QStringLiteral("rightshoulder")},
        {QStringLiteral("lt"), QStringLiteral("lefttrigger")},
        {QStringLiteral("rt"), QStringLiteral("righttrigger")},
        {QStringLiteral("l2"), QStringLiteral("lefttrigger")},
        {QStringLiteral("r2"), QStringLiteral("righttrigger")},
        {QStringLiteral("zl"), QStringLiteral("lefttrigger")},
        {QStringLiteral("zr"), QStringLiteral("righttrigger")},
        {QStringLiteral("ls"), QStringLiteral("leftstick")},
        {QStringLiteral("rs"), QStringLiteral("rightstick")},
        {QStringLiteral("l3"), QStringLiteral("leftstick")},
        {QStringLiteral("r3"), QStringLiteral("rightstick")},
        {QStringLiteral("up"), QStringLiteral("dpup")},
        {QStringLiteral("down"), QStringLiteral("dpdown")},
        {QStringLiteral("left"), QStringLiteral("dpleft")},
        {QStringLiteral("right"), QStringLiteral("dpright")},
        {QStringLiteral("south"), QStringLiteral("a")},
        {QStringLiteral("east"), QStringLiteral("b")},
        {QStringLiteral("west"), QStringLiteral("x")},
        {QStringLiteral("north"), QStringLiteral("y")},
    };
    return table;
}

constexpr int GlyphColumns = 19;

const std::array<const char *, GlyphColumns> glyphButtons = {
    "a",
    "b",
    "x",
    "y",
    "leftshoulder",
    "rightshoulder",
    "lefttrigger",
    "righttrigger",
    "back",
    "start",
    "guide",
    "leftstick",
    "rightstick",
    "misc1",
    "touchpad",
    "leftpaddle1",
    "rightpaddle1",
    "leftpaddle2",
    "rightpaddle2",
};

struct TypeGlyphs
{
    const char *type;
    std::array<const char *, GlyphColumns> labels;
};

constexpr std::array<TypeGlyphs, 5> glyphTable = {{
    {"xbox", {"A", "B", "X", "Y", "LB", "RB", "LT", "RT", "View", "Menu", "Xbox", "LS", "RS", "Share", nullptr, "P3", "P1", "P4", "P2"}},
    {"playstation",
        {"✕", "○", "□", "△", "L1", "R1", "L2", "R2", "Create", "Options", "PS", "L3", "R3", "Mute", "Touchpad", "LB", "RB", "LFn", "RFn"}},
    {"nintendo",
        {"B", "A", "Y", "X", "L", "R", "ZL", "ZR", "−", "+", "Home", "LS", "RS", "Capture", nullptr, "SL", "SR", nullptr, nullptr}},
    {"steamdeck", {"A", "B", "X", "Y", "L1", "R1", "L2", "R2", "View", "Menu", "Steam", "L3", "R3", "…", nullptr, "L4", "R4", "L5", "R5"}},
    {"generic",
        {"A", "B", "X", "Y", "LB", "RB", "LT", "RT", "Back", "Start", "Guide", "LS", "RS", nullptr, nullptr, nullptr, nullptr, nullptr,
            nullptr}},
}};

const QHash<QString, QString> &directionGlyphs()
{
    static const QHash<QString, QString> table = {
        {QStringLiteral("dpup"), QStringLiteral("↑")},
        {QStringLiteral("dpdown"), QStringLiteral("↓")},
        {QStringLiteral("dpleft"), QStringLiteral("←")},
        {QStringLiteral("dpright"), QStringLiteral("→")},
    };
    return table;
}

const TypeGlyphs &glyphsForType(const QString &controllerType)
{
    for (const TypeGlyphs &entry : glyphTable) {
        if (controllerType == QLatin1String(entry.type)) {
            return entry;
        }
    }
    return glyphTable.back();
}

const char *labelFor(const TypeGlyphs &glyphs, const QString &button)
{
    for (int column = 0; column < GlyphColumns; ++column) {
        if (button == QLatin1String(glyphButtons.at(column))) {
            return glyphs.labels.at(column);
        }
    }
    return nullptr;
}
}

namespace GamepadButtons
{
QStringList names()
{
    QStringList all = sdlOrder();
    all.append(QStringLiteral("lefttrigger"));
    all.append(QStringLiteral("righttrigger"));
    return all;
}

QString nameForSdlButton(int sdlButton)
{
    const auto &order = sdlOrder();
    if (sdlButton < 0 || sdlButton >= order.size()) {
        return {};
    }
    return order.at(sdlButton);
}

int sdlButtonForName(const QString &name)
{
    return sdlOrder().indexOf(name);
}

QString canonicalName(const QString &nameOrAlias)
{
    const QString key = nameOrAlias.trimmed().toLower();
    const auto alias = aliases().constFind(key);
    const QString resolved = alias != aliases().constEnd() ? alias.value() : key;
    return isKnown(resolved) ? resolved : QString();
}

bool isKnown(const QString &name)
{
    return sdlOrder().contains(name) || name == QLatin1String("lefttrigger") || name == QLatin1String("righttrigger");
}

QStringList controllerTypes()
{
    return {QStringLiteral("xbox"), QStringLiteral("playstation"), QStringLiteral("nintendo"), QStringLiteral("steamdeck"),
        QStringLiteral("generic")};
}

QString glyph(const QString &controllerType, const QString &button)
{
    const QString canonical = canonicalName(button);
    if (canonical.isEmpty()) {
        return {};
    }
    const auto direction = directionGlyphs().constFind(canonical);
    if (direction != directionGlyphs().constEnd()) {
        return direction.value();
    }
    const char *label = labelFor(glyphsForType(controllerType), canonical);
    return label ? QString::fromUtf8(label) : canonical.toUpper();
}

bool isValveController(quint16 vendor, quint16 product)
{
    return vendor == SteamDedupe::ValveVendor && product != SteamDedupe::SteamVirtualProduct;
}

QString typeForSdl(int sdlType, quint16 vendor, quint16 product)
{
    if (isValveController(vendor, product)) {
        return QStringLiteral("steamdeck");
    }
    if (vendor == SteamDedupe::ValveVendor) {
        return QStringLiteral("xbox");
    }
    switch (sdlType) {
    case SDL_GAMEPAD_TYPE_XBOX360:
    case SDL_GAMEPAD_TYPE_XBOXONE:
        return QStringLiteral("xbox");
    case SDL_GAMEPAD_TYPE_PS3:
    case SDL_GAMEPAD_TYPE_PS4:
    case SDL_GAMEPAD_TYPE_PS5:
        return QStringLiteral("playstation");
    case SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_PRO:
    case SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_LEFT:
    case SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_RIGHT:
    case SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_PAIR:
    case SDL_GAMEPAD_TYPE_GAMECUBE:
        return QStringLiteral("nintendo");
    default:
        return QStringLiteral("generic");
    }
}
}
