#pragma once

#include <QList>
#include <QString>

namespace TranscriptText
{
struct Action
{
    enum Kind
    {
        Insert,
        NewLine,
        DeletePrevious,
    };
    Kind kind = Insert;
    QString text;

    bool operator==(const Action &other) const = default;
};

QList<Action> applyCommands(const QString &transcript);
QList<Action> plain(const QString &transcript);
QString fitToContext(const QString &text, const QString &textBeforeCursor);
}
