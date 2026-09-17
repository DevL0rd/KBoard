#include "predictor.h"

#include "languagedata.h"
#include "textanalysis.h"
#include "usermodel.h"
#include "wordscorer.h"

#include <algorithm>

namespace
{
constexpr double BaseBigramWeight = 0.3;
constexpr double UserBigramWeight = 0.8;
constexpr double BaseTrigramWeight = 0.5;
constexpr double UserTrigramWeight = 1.2;
constexpr double FrequencyTieBreak = 1e-4;

void addWeighted(QHash<QString, double> &target, const QHash<QString, double> &source, double weight)
{
    for (auto it = source.cbegin(); it != source.cend(); ++it) {
        target[it.key()] += weight * it.value();
    }
}
}

Predictor::Predictor(const WordScorer &scorer)
    : m_scorer(scorer)
{ }

Predictor::Distribution Predictor::fromBase(const QString &context) const
{
    Distribution distribution;
    const LanguageData &data = m_scorer.data();
    if (const auto *followers = data.ngrams.followers(context)) {
        for (const NGramModel::Follower &follower : *followers) {
            distribution.insert(data.lexicon.entry(follower.entry).key, follower.probability);
        }
    }
    return distribution;
}

Predictor::Distribution Predictor::fromUser(const QString &context) const
{
    Distribution distribution;
    const auto *followers = m_scorer.user().followers(context);
    if (!followers) {
        return distribution;
    }
    double total = 0;
    for (int count : *followers) {
        total += count;
    }
    const double confidence = std::min(1.0, total / 2.0);
    for (auto it = followers->cbegin(); it != followers->cend(); ++it) {
        distribution.insert(it.key(), (it.value() + 0.5) / (total + 1.0) * confidence);
    }
    return distribution;
}

Predictor::Distribution Predictor::mixture(const QStringList &previousKeys) const
{
    Distribution scores;
    const QString &last = previousKeys.last();
    addWeighted(scores, fromBase(last), BaseBigramWeight);
    addWeighted(scores, fromUser(last), UserBigramWeight);
    if (previousKeys.size() >= 2) {
        const QString pair = previousKeys.at(previousKeys.size() - 2) + u' ' + last;
        addWeighted(scores, fromBase(pair), BaseTrigramWeight);
        addWeighted(scores, fromUser(pair), UserTrigramWeight);
    }
    return scores;
}

QVector<Suggestion> Predictor::predict(const QStringList &previousKeys, bool sentenceStart, int limit) const
{
    QVector<Suggestion> ordered;
    if (previousKeys.isEmpty()) {
        return ordered;
    }
    const Distribution scores = mixture(previousKeys);
    const Lexicon &lexicon = m_scorer.data().lexicon;
    for (auto it = scores.cbegin(); it != scores.cend(); ++it) {
        if (m_scorer.user().isBlocked(it.key())) {
            continue;
        }
        const int entry = lexicon.find(it.key());
        const double tieBreak = entry >= 0 ? FrequencyTieBreak * lexicon.entry(entry).logFrequency : 0.0;
        ordered.append(Suggestion {QString(), it.key(), SuggestionKind::Prediction, it.value() + tieBreak, 0});
    }
    std::sort(ordered.begin(), ordered.end(), [](const Suggestion &a, const Suggestion &b) { return a.score > b.score; });
    if (ordered.size() > limit) {
        ordered.resize(limit);
    }
    for (Suggestion &suggestion : ordered) {
        const QString form = m_scorer.displayForm(suggestion.key);
        suggestion.text = sentenceStart ? TextAnalysis::capitalise(form) : form;
    }
    return ordered;
}
