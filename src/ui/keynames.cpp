#include "keynames.h"
#include "keycodes.h"

#include <QHash>

#include <linux/input-event-codes.h>

namespace
{
struct Symbol
{
    uint code;
    bool shifted;
};

const QHash<QChar, Symbol> &symbols()
{
    static const QHash<QChar, Symbol> table = {
        {u'-', {KEY_MINUS, false}},
        {u'_', {KEY_MINUS, true}},
        {u'=', {KEY_EQUAL, false}},
        {u'+', {KEY_EQUAL, true}},
        {u'[', {KEY_LEFTBRACE, false}},
        {u'{', {KEY_LEFTBRACE, true}},
        {u']', {KEY_RIGHTBRACE, false}},
        {u'}', {KEY_RIGHTBRACE, true}},
        {u';', {KEY_SEMICOLON, false}},
        {u':', {KEY_SEMICOLON, true}},
        {u'\'', {KEY_APOSTROPHE, false}},
        {u'"', {KEY_APOSTROPHE, true}},
        {u'`', {KEY_GRAVE, false}},
        {u'~', {KEY_GRAVE, true}},
        {u'\\', {KEY_BACKSLASH, false}},
        {u'|', {KEY_BACKSLASH, true}},
        {u',', {KEY_COMMA, false}},
        {u'<', {KEY_COMMA, true}},
        {u'.', {KEY_DOT, false}},
        {u'>', {KEY_DOT, true}},
        {u'/', {KEY_SLASH, false}},
        {u'?', {KEY_SLASH, true}},
        {u' ', {KEY_SPACE, false}},
        {u'!', {KEY_1, true}},
        {u'@', {KEY_2, true}},
        {u'#', {KEY_3, true}},
        {u'$', {KEY_4, true}},
        {u'%', {KEY_5, true}},
        {u'^', {KEY_6, true}},
        {u'&', {KEY_7, true}},
        {u'*', {KEY_8, true}},
        {u'(', {KEY_9, true}},
        {u')', {KEY_0, true}},
    };
    return table;
}
}

KeyNames::KeyNames(QObject *parent)
    : QObject(parent)
{ }

uint KeyNames::evdev(const QString &name)
{
    if (name.size() == 1 && symbols().contains(name.at(0))) {
        return symbols().value(name.at(0)).code;
    }
    return KeyCodes::evdevForName(name.toLower());
}

bool KeyNames::needsShift(const QString &text)
{
    if (text.size() != 1) {
        return false;
    }
    const QChar ch = text.at(0);
    return symbols().value(ch, Symbol {0, false}).shifted || (ch.isLetter() && ch.isUpper());
}

uint KeyNames::modifierMask(const QString &modifier)
{
    return KeyCodes::modifierForName(modifier);
}
