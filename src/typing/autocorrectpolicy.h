#pragma once

#include "suggestion.h"

#include <QVector>

class WordScorer;
struct WordQuery;

class AutocorrectPolicy
{
public:
    enum Strength
    {
        Off = 0,
        Mild = 1,
        Normal = 2,
        Aggressive = 3,
    };

    AutocorrectPolicy(const WordScorer &scorer, int strength);

    QString decide(const WordQuery &query, const QVector<Suggestion> &ranked) const;

    static double maxCost(int strength, int length);

private:
    QString contractionFix(const WordQuery &query, const QVector<Suggestion> &ranked) const;
    QString rareWordFix(const WordQuery &query, const QVector<Suggestion> &ranked) const;
    QString typoFix(const WordQuery &query, const QVector<Suggestion> &ranked) const;
    bool userOwns(const WordQuery &query) const;
    bool isConfident(const Suggestion &best, const Suggestion *second) const;
    static bool looksUnfinished(const WordQuery &query, const QVector<Suggestion> &ranked, const Suggestion &best);

    const WordScorer &m_scorer;
    int m_strength;
};
