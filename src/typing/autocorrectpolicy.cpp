#include "autocorrectpolicy.h"

#include "languagedata.h"
#include "textanalysis.h"
#include "usermodel.h"
#include "wordranker.h"
#include "wordscorer.h"

#include <algorithm>

namespace
{
constexpr double ContractionRatio = 35.0;
constexpr double RareWordMaxCost = 1.0;
constexpr float CommonWordFrequency = 10.0f;
constexpr double MildMargin = 1.0;
constexpr double MildMinimumPrior = 8.0;
constexpr double CaseVariantAllowance = 0.5;
constexpr double MaxMargin = 100.0;

const Suggestion *firstCorrection(const WordQuery &query, const QVector<Suggestion> &ranked, const UserModel &user, int skip)
{
    for (const Suggestion &suggestion : ranked) {
        if (suggestion.kind != SuggestionKind::Correction || user.isRejected(query.key, suggestion.key)) {
            continue;
        }
        if (skip-- == 0) {
            return &suggestion;
        }
    }
    return nullptr;
}
}

AutocorrectPolicy::AutocorrectPolicy(const WordScorer &scorer, int strength)
    : m_scorer(scorer)
    , m_strength(strength)
{ }

double AutocorrectPolicy::maxCost(int strength, int length)
{
    static constexpr double table[4][4] = {
        {0, 0, 0, 0},
        {0, 0.7, 1.0, 1.4},
        {0.65, 1.05, 1.6, 2.1},
        {0.75, 1.4, 2.1, 2.6},
    };
    const int bucket = length <= 2 ? 0 : length <= 4 ? 1 : length <= 7 ? 2 : 3;
    return table[std::clamp(strength, 0, 3)][bucket];
}

bool AutocorrectPolicy::userOwns(const WordQuery &query) const
{
    return m_scorer.user().isKnown(query.key);
}

QString AutocorrectPolicy::decide(const WordQuery &query, const QVector<Suggestion> &ranked) const
{
    if (m_strength <= Off || query.key.isEmpty()) {
        return QString();
    }
    if (!query.spelled) {
        return typoFix(query, ranked);
    }
    if (userOwns(query)) {
        return QString();
    }
    QString fix = m_strength >= Normal ? contractionFix(query, ranked) : QString();
    if (fix.isEmpty() && m_strength >= Aggressive) {
        fix = rareWordFix(query, ranked);
    }
    return fix;
}

QString AutocorrectPolicy::contractionFix(const WordQuery &query, const QVector<Suggestion> &ranked) const
{
    const Lexicon &lexicon = m_scorer.data().lexicon;
    const double typedCount = query.entry >= 0 ? double(lexicon.entry(query.entry).count) : 0.0;
    for (const Suggestion &suggestion : ranked) {
        if (!suggestion.key.contains(u'\'') || QString(suggestion.key).remove(u'\'') != query.key
            || m_scorer.user().isRejected(query.key, suggestion.key)) {
            continue;
        }
        const int entry = lexicon.find(suggestion.key);
        if (entry >= 0 && double(lexicon.entry(entry).count) >= ContractionRatio * std::max(typedCount, 1.0)) {
            return suggestion.text;
        }
    }
    return QString();
}

QString AutocorrectPolicy::rareWordFix(const WordQuery &query, const QVector<Suggestion> &ranked) const
{
    if (query.entry >= 0) {
        return QString();
    }
    const Suggestion *best = firstCorrection(query, ranked, m_scorer.user(), 0);
    if (!best || best->cost > RareWordMaxCost) {
        return QString();
    }
    const int entry = m_scorer.data().lexicon.find(best->key);
    return entry >= 0 && m_scorer.data().lexicon.entry(entry).logFrequency >= CommonWordFrequency ? best->text : QString();
}

bool AutocorrectPolicy::isConfident(const Suggestion &best, const Suggestion *second) const
{
    const double margin = second ? best.score - second->score : MaxMargin;
    if (m_strength == Normal) {
        return margin >= 0.0;
    }
    if (m_strength != Mild) {
        return true;
    }
    const int entry = m_scorer.data().lexicon.find(best.key);
    const double prior = entry >= 0 ? double(m_scorer.data().lexicon.entry(entry).logFrequency) : m_scorer.userPrior(best.key);
    return margin >= MildMargin && prior >= MildMinimumPrior;
}

bool AutocorrectPolicy::looksUnfinished(const WordQuery &query, const QVector<Suggestion> &ranked, const Suggestion &best)
{
    const Suggestion &top = ranked.first();
    return top.kind == SuggestionKind::Completion && top.key.size() - query.key.size() <= query.key.size() && top.score > best.score;
}

QString AutocorrectPolicy::typoFix(const WordQuery &query, const QVector<Suggestion> &ranked) const
{
    const Suggestion *best = firstCorrection(query, ranked, m_scorer.user(), 0);
    if (!best) {
        return QString();
    }
    const double allowed = best->key == query.key ? CaseVariantAllowance : maxCost(m_strength, int(query.key.size()));
    if (best->cost > allowed || looksUnfinished(query, ranked, *best)) {
        return QString();
    }
    return isConfident(*best, firstCorrection(query, ranked, m_scorer.user(), 1)) ? best->text : QString();
}
