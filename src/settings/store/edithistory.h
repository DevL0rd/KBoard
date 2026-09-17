#pragma once

#include <QList>
#include <QString>
#include <QVariant>

#include <optional>

struct EditChange
{
    QString name;
    QVariant before;
};

class EditHistory
{
public:
    static constexpr qint64 BurstMs = 700;
    static constexpr qsizetype MaxSteps = 200;

    void record(const QString &name, const QVariant &before, qint64 nowMs);
    void recordStep(const QList<EditChange> &changes);
    std::optional<QList<EditChange>> undo();
    void breakBurst();
    bool canUndo() const;
    qsizetype size() const;

private:
    QList<QList<EditChange>> m_steps;
    QString m_burstName;
    qint64 m_burstMs = -1;
};
