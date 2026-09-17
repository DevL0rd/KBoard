#include "buttonsemantics.h"

#include <algorithm>

ButtonSemantics::ButtonSemantics(QObject *parent)
    : QObject(parent)
{ }

bool ButtonSemantics::setChord(const QString &chord, const QSet<QString> &held)
{
    const bool valid = m_matcher.setChord(chord);
    m_matcher.reset();
    m_chordConsumed.clear();
    m_suppressed.clear();
    for (const QString &button : held) {
        m_matcher.press(button);
    }
    return valid;
}

QStringList ButtonSemantics::chord() const
{
    return m_matcher.buttons();
}

QString ButtonSemantics::chordError() const
{
    return m_matcher.errorString();
}

void ButtonSemantics::press(const QString &button)
{
    if (!m_matcher.press(button)) {
        Q_EMIT pressed(button);
        return;
    }
    const QStringList members = m_matcher.buttons();
    m_chordConsumed.unite(QSet<QString>(members.cbegin(), members.cend()));
    m_suppressed.insert(button);
    Q_EMIT chordActivated();
}

void ButtonSemantics::release(const QString &button)
{
    m_matcher.release(button);
    const bool consumed = m_chordConsumed.remove(button);
    if (m_suppressed.remove(button)) {
        return;
    }
    Q_EMIT released(button);
    if (!consumed) {
        Q_EMIT tapped(button);
    }
}

void ButtonSemantics::releaseAll(const QSet<QString> &held)
{
    QStringList ordered(held.cbegin(), held.cend());
    std::sort(ordered.begin(), ordered.end());
    for (const QString &button : std::as_const(ordered)) {
        if (!m_suppressed.contains(button)) {
            Q_EMIT released(button);
        }
    }
    m_matcher.reset();
    m_chordConsumed.clear();
    m_suppressed.clear();
}
