#include "chordmatcher.h"

#include "gamepadbuttons.h"

ChordMatcher::Parsed ChordMatcher::parse(const QString &chord)
{
    Parsed result;
    const QStringList parts = chord.split(QLatin1Char('+'), Qt::SkipEmptyParts);
    if (parts.isEmpty()) {
        result.error = QStringLiteral("The controller chord is empty");
        return result;
    }
    for (const QString &part : parts) {
        const QString trimmed = part.trimmed();
        if (trimmed.isEmpty()) {
            continue;
        }
        const QString canonical = GamepadButtons::canonicalName(trimmed);
        if (canonical.isEmpty()) {
            result.buttons.clear();
            result.error = QStringLiteral("Unknown controller button \"%1\" in chord \"%2\"").arg(trimmed, chord);
            return result;
        }
        if (!result.buttons.contains(canonical)) {
            result.buttons.append(canonical);
        }
    }
    if (result.buttons.isEmpty()) {
        result.error = QStringLiteral("The controller chord is empty");
    }
    return result;
}

bool ChordMatcher::setChord(const QString &chord)
{
    const Parsed parsed = parse(chord);
    m_buttons = parsed.buttons;
    m_error = parsed.error;
    m_fired = false;
    return m_error.isEmpty();
}

QStringList ChordMatcher::buttons() const
{
    return m_buttons;
}

QString ChordMatcher::errorString() const
{
    return m_error;
}

bool ChordMatcher::isValid() const
{
    return !m_buttons.isEmpty();
}

bool ChordMatcher::press(const QString &button)
{
    m_held.insert(button);
    if (m_fired || !isValid() || !m_buttons.contains(button)) {
        return false;
    }
    for (const QString &required : std::as_const(m_buttons)) {
        if (!m_held.contains(required)) {
            return false;
        }
    }
    m_fired = true;
    return true;
}

void ChordMatcher::release(const QString &button)
{
    m_held.remove(button);
    if (m_buttons.contains(button)) {
        m_fired = false;
    }
}

void ChordMatcher::reset()
{
    m_held.clear();
    m_fired = false;
}
