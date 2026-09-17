#include "edithistory.h"

void EditHistory::record(const QString &name, const QVariant &before, qint64 nowMs)
{
    const bool inBurst = !m_steps.isEmpty() && m_burstMs >= 0 && name == m_burstName && nowMs - m_burstMs < BurstMs;
    m_burstName = name;
    m_burstMs = nowMs;
    if (inBurst) {
        return;
    }
    m_steps.append({EditChange {name, before}});
    if (m_steps.size() > MaxSteps) {
        m_steps.removeFirst();
    }
}

void EditHistory::recordStep(const QList<EditChange> &changes)
{
    breakBurst();
    if (changes.isEmpty()) {
        return;
    }
    m_steps.append(changes);
    if (m_steps.size() > MaxSteps) {
        m_steps.removeFirst();
    }
}

std::optional<QList<EditChange>> EditHistory::undo()
{
    breakBurst();
    if (m_steps.isEmpty()) {
        return std::nullopt;
    }
    return m_steps.takeLast();
}

void EditHistory::breakBurst()
{
    m_burstName.clear();
    m_burstMs = -1;
}

bool EditHistory::canUndo() const
{
    return !m_steps.isEmpty();
}

qsizetype EditHistory::size() const
{
    return m_steps.size();
}
