#pragma once

#include "suggestion.h"

#include <QStringList>
#include <QVector>

class Corrector;
class WordScorer;

struct WordQuery
{
    QString typed;
    QString key;
    int entry = -1;
    bool known = false;
    bool spelled = false;
    bool caseVariant = false;
    QStringList previousKeys;
};

class WordRanker
{
public:
    WordRanker(const WordScorer &scorer, const Corrector &corrector);

    WordQuery query(const QString &typed, const QStringList &previousKeys) const;
    QVector<Suggestion> rank(const WordQuery &query) const;
    QVector<Suggestion> completions(const WordQuery &query) const;

    static double searchCost(int length);

private:
    class Pool;

    void addCorrections(const WordQuery &query, Pool &pool) const;
    void addUserCorrections(const WordQuery &query, Pool &pool) const;
    void addCompletions(const WordQuery &query, Pool &pool) const;
    void addUserCompletions(const WordQuery &query, Pool &pool) const;
    void addTyped(const WordQuery &query, Pool &pool) const;
    QVector<QString> unlistedUserKeys() const;
    Suggestion make(const WordQuery &query, const QString &key, const QString &form, SuggestionKind kind, double score) const;

    const WordScorer &m_scorer;
    const Corrector &m_corrector;
};
