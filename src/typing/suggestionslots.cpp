#include "suggestionslots.h"

#include "textanalysis.h"
#include "wordranker.h"

#include <QStringList>

namespace SuggestionSlots
{
namespace
{
Suggestion bestExcluding(const QVector<Suggestion> &ranked, const QStringList &excludedKeys)
{
    for (const Suggestion &suggestion : ranked) {
        if (!excludedKeys.contains(suggestion.key)) {
            return suggestion;
        }
    }
    return Suggestion {};
}

Suggestion typedLiteral(const WordQuery &query)
{
    return Suggestion {query.typed, query.key, SuggestionKind::Typed, 0, 0};
}

QVector<Suggestion> arranged(const Suggestion &left, const Suggestion &middle, const Suggestion &right)
{
    if (middle.text.isEmpty()) {
        return {};
    }
    return {left, middle, right};
}
}

QVector<Suggestion> forWord(
    const WordQuery &query, const QVector<Suggestion> &ranked, const QString &autocorrection, const QString &expansion)
{
    if (!expansion.isEmpty()) {
        const Suggestion middle {expansion, QString(), SuggestionKind::Expansion, 0, 0};
        return arranged(typedLiteral(query), middle, bestExcluding(ranked, {query.key}));
    }
    if (!autocorrection.isEmpty()) {
        const Suggestion middle {autocorrection, TextAnalysis::normalizeKey(autocorrection), SuggestionKind::Correction, 0, 0};
        return arranged(typedLiteral(query), middle, bestExcluding(ranked, {query.key, middle.key}));
    }
    const Suggestion middle = bestExcluding(ranked, {});
    if (!query.spelled && middle.key != query.key) {
        return arranged(typedLiteral(query), middle, bestExcluding(ranked, {query.key, middle.key}));
    }
    Suggestion left = bestExcluding(ranked, {middle.key});
    const Suggestion right = bestExcluding(ranked, {middle.key, left.key});
    const bool typedShown = middle.key == query.key || left.key == query.key || right.key == query.key;
    if (query.spelled && !typedShown) {
        left = typedLiteral(query);
    }
    return arranged(left, middle, right);
}

QVector<Suggestion> forRanked(const QVector<Suggestion> &ranked)
{
    return arranged(ranked.value(1), ranked.value(0), ranked.value(2));
}
}
