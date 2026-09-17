#pragma once

#include "suggestion.h"

#include <QHash>
#include <QStringList>
#include <QVector>

class WordScorer;

class Predictor
{
public:
    explicit Predictor(const WordScorer &scorer);

    QVector<Suggestion> predict(const QStringList &previousKeys, bool sentenceStart, int limit) const;

private:
    using Distribution = QHash<QString, double>;

    Distribution fromBase(const QString &context) const;
    Distribution fromUser(const QString &context) const;
    Distribution mixture(const QStringList &previousKeys) const;

    const WordScorer &m_scorer;
};
