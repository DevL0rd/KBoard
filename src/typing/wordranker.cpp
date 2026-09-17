#include "wordranker.h"

#include "autocorrectpolicy.h"
#include "corrector.h"
#include "languagedata.h"
#include "textanalysis.h"
#include "usermodel.h"
#include "wordscorer.h"

#include <QHash>

#include <algorithm>

namespace
{
constexpr double CorrectionCostWeight = 7.0;
constexpr double CaseVariantCost = 0.25;
constexpr double CompletionBasePenalty = 1.2;
constexpr double CompletionCharPenalty = 0.55;
constexpr double ExactBonus = 2.5;
constexpr double UnlistedWordPrior = 6.0;
constexpr int CompletionLimit = 12;

double completionPenalty(const QString &completionKey, const QString &key)
{
    return CompletionBasePenalty + CompletionCharPenalty * double(completionKey.size() - key.size());
}

double correctionPenalty(double cost)
{
    return cost == 0 ? -ExactBonus : CorrectionCostWeight * cost;
}

SuggestionKind correctionKind(double cost)
{
    return cost == 0 ? SuggestionKind::Typed : SuggestionKind::Correction;
}
}

class WordRanker::Pool
{
public:
    void consider(const Suggestion &suggestion)
    {
        auto it = m_items.find(suggestion.key);
        if (it == m_items.end()) {
            m_items.insert(suggestion.key, suggestion);
        } else if (suggestion.score > it->score) {
            *it = suggestion;
        }
    }

    bool contains(const QString &key) const { return m_items.contains(key); }

    QVector<Suggestion> sorted() const
    {
        QVector<Suggestion> result(m_items.cbegin(), m_items.cend());
        std::sort(result.begin(), result.end(), [](const Suggestion &a, const Suggestion &b) { return a.score > b.score; });
        return result;
    }

private:
    QHash<QString, Suggestion> m_items;
};

WordRanker::WordRanker(const WordScorer &scorer, const Corrector &corrector)
    : m_scorer(scorer)
    , m_corrector(corrector)
{ }

double WordRanker::searchCost(int length)
{
    return length <= 1 ? 0.0 : AutocorrectPolicy::maxCost(AutocorrectPolicy::Aggressive, length);
}

WordQuery WordRanker::query(const QString &typed, const QStringList &previousKeys) const
{
    WordQuery query;
    query.typed = typed;
    query.key = TextAnalysis::normalizeKey(typed);
    query.entry = m_scorer.data().lexicon.find(query.key);
    query.known = m_scorer.isKnown(query.key);
    const bool lowercase = TextAnalysis::casingOf(typed) == TextAnalysis::Casing::Lower;
    query.caseVariant
        = query.entry >= 0 && lowercase && !m_scorer.user().isKnown(query.key) && m_scorer.data().lexicon.entry(query.entry).form != typed;
    query.spelled = !query.caseVariant && (query.known || m_scorer.data().spell(typed));
    query.previousKeys = previousKeys;
    return query;
}

Suggestion WordRanker::make(const WordQuery &query, const QString &key, const QString &form, SuggestionKind kind, double score) const
{
    return Suggestion {TextAnalysis::applyCasing(query.typed, form, false), key, kind, score, 0};
}

void WordRanker::addCorrections(const WordQuery &query, Pool &pool) const
{
    const double limit = searchCost(query.key.size());
    if (limit <= 0) {
        return;
    }
    const LanguageData &data = m_scorer.data();
    for (const Corrector::Match &match : m_corrector.search(data.lexicon, query.key, limit)) {
        const QString &key = data.lexicon.entry(match.entry).key;
        if (m_scorer.user().isBlocked(key)) {
            continue;
        }
        const double cost = match.cost == 0 && query.caseVariant ? CaseVariantCost : match.cost;
        const double score = m_scorer.contextualPrior(query.previousKeys, key, match.entry) - correctionPenalty(cost);
        Suggestion suggestion = make(query, key, m_scorer.displayForm(key), correctionKind(cost), score);
        suggestion.cost = cost;
        pool.consider(suggestion);
    }
}

QVector<QString> WordRanker::unlistedUserKeys() const
{
    QVector<QString> keys;
    const UserModel &user = m_scorer.user();
    for (auto it = user.words().cbegin(); it != user.words().cend(); ++it) {
        if (user.isKnown(it.key()) && m_scorer.data().lexicon.find(it.key()) < 0) {
            keys.append(it.key());
        }
    }
    return keys;
}

void WordRanker::addUserCorrections(const WordQuery &query, Pool &pool) const
{
    const double limit = searchCost(query.key.size());
    if (limit <= 0) {
        return;
    }
    for (const QString &key : unlistedUserKeys()) {
        const double cost = std::abs(key.size() - query.key.size()) <= 3 ? m_corrector.distance(query.key, key, limit) : limit + 1;
        if (cost <= limit) {
            const double score = m_scorer.contextualPrior(query.previousKeys, key, -1) - correctionPenalty(cost);
            Suggestion suggestion = make(query, key, m_scorer.displayForm(key), correctionKind(cost), score);
            suggestion.cost = cost;
            pool.consider(suggestion);
        }
    }
}

void WordRanker::addUserCompletions(const WordQuery &query, Pool &pool) const
{
    for (const QString &key : unlistedUserKeys()) {
        if (key.size() > query.key.size() && key.startsWith(query.key)) {
            const double score = m_scorer.contextualPrior(query.previousKeys, key, -1) - completionPenalty(key, query.key);
            pool.consider(make(query, key, m_scorer.displayForm(key), SuggestionKind::Completion, score));
        }
    }
}

void WordRanker::addCompletions(const WordQuery &query, Pool &pool) const
{
    const LanguageData &data = m_scorer.data();
    for (int entry : data.lexicon.prefixMatches(query.key, CompletionLimit, 1)) {
        const QString &key = data.lexicon.entry(entry).key;
        if (m_scorer.user().isBlocked(key)) {
            continue;
        }
        const double score = m_scorer.contextualPrior(query.previousKeys, key, entry) - completionPenalty(key, query.key);
        pool.consider(make(query, key, m_scorer.displayForm(key), SuggestionKind::Completion, score));
    }
}

void WordRanker::addTyped(const WordQuery &query, Pool &pool) const
{
    if (query.caseVariant && !pool.contains(query.key)) {
        const double score = m_scorer.contextualPrior(query.previousKeys, query.key, query.entry) - correctionPenalty(CaseVariantCost);
        Suggestion suggestion = make(query, query.key, m_scorer.displayForm(query.key), SuggestionKind::Correction, score);
        suggestion.cost = CaseVariantCost;
        pool.consider(suggestion);
    }
    if (query.spelled && !pool.contains(query.key)) {
        const double prior = query.known ? m_scorer.prior(query.key, query.entry) : UnlistedWordPrior;
        pool.consider(Suggestion {query.typed, query.key, SuggestionKind::Typed, prior + ExactBonus, 0});
    }
}

QVector<Suggestion> WordRanker::rank(const WordQuery &query) const
{
    Pool pool;
    addCorrections(query, pool);
    addUserCorrections(query, pool);
    addCompletions(query, pool);
    addUserCompletions(query, pool);
    addTyped(query, pool);
    return pool.sorted();
}

QVector<Suggestion> WordRanker::completions(const WordQuery &query) const
{
    Pool pool;
    addCompletions(query, pool);
    addUserCompletions(query, pool);
    if (query.known) {
        pool.consider(
            make(query, query.key, m_scorer.displayForm(query.key), SuggestionKind::Typed, m_scorer.prior(query.key, query.entry)));
    }
    return pool.sorted();
}
