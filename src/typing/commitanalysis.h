#pragma once

#include <QString>
#include <QStringList>

#include <optional>

class WordScorer;

struct CommittedWord
{
    QString form;
    QString key;
    QStringList previousKeys;
};

std::optional<CommittedWord> analyseCommit(
    const WordScorer &scorer, const QString &textBefore, const QString &partialWord, const QString &word);
