#pragma once

#include <QList>
#include <QString>

namespace KeyCodes
{
uint evdevForName(const QString &name);
uint modifierForName(const QString &name);

struct KeyEvent
{
    uint code;
    bool pressed;

    bool operator==(const KeyEvent &other) const = default;
};

QList<KeyEvent> chord(uint modifiers, uint code);
}
