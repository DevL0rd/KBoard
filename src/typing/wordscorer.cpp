#include "wordscorer.h"

#include "languagedata.h"
#include "textanalysis.h"
#include "usermodel.h"

#include <cmath>
#include <limits>

WordScorer::WordScorer(const LanguageData &data, const UserModel &user)
    : m_data(data)
    , m_user(user)
{ }

double WordScorer::userPrior(const QString &key) const
{
    const UserModel::Word *word = m_user.word(key);
    if (!word || !m_user.isKnown(key)) {
        return 0;
    }
    return UserBaseline + 1.5 * std::log1p(double(word->count)) + (word->pinned ? 1.0 : 0.0);
}

double WordScorer::prior(const QString &key, int entry) const
{
    if (m_user.isBlocked(key)) {
        return -std::numeric_limits<double>::infinity();
    }
    const double lexiconPrior = entry >= 0 ? double(m_data.lexicon.entry(entry).logFrequency) : 0.0;
    const double learned = userPrior(key);
    if (learned > 0) {
        return std::max(lexiconPrior, UserBaseline) + (learned - UserBaseline) + 1.0;
    }
    return lexiconPrior;
}

double WordScorer::baseBoost(const QString &context, int entry, double weight) const
{
    const auto *followers = m_data.ngrams.followers(context);
    if (!followers || entry < 0) {
        return 0;
    }
    for (const NGramModel::Follower &follower : *followers) {
        if (follower.entry == entry) {
            return weight * std::log1p(20.0 * follower.probability);
        }
    }
    return 0;
}

double WordScorer::userBoost(const QString &context, const QString &key) const
{
    const auto *followers = m_user.followers(context);
    return followers ? 2.0 * std::log1p(double(followers->value(key))) : 0.0;
}

double WordScorer::contextBoost(const QStringList &previousKeys, const QString &key, int entry) const
{
    if (previousKeys.isEmpty()) {
        return 0;
    }
    const QString &last = previousKeys.last();
    double boost = baseBoost(last, entry, 2.5) + userBoost(last, key);
    if (previousKeys.size() >= 2) {
        const QString pair = previousKeys.at(previousKeys.size() - 2) + u' ' + last;
        boost += baseBoost(pair, entry, 2.0) + userBoost(pair, key);
    }
    return boost;
}

double WordScorer::contextualPrior(const QStringList &previousKeys, const QString &key, int entry) const
{
    return prior(key, entry) + contextBoost(previousKeys, key, entry);
}

QString WordScorer::displayForm(const QString &key) const
{
    if (const UserModel::Word *word = m_user.word(key)) {
        return word->form;
    }
    const int entry = m_data.lexicon.find(key);
    return entry >= 0 ? m_data.lexicon.entry(entry).form : key;
}

bool WordScorer::isKnown(const QString &key) const
{
    return !m_user.isBlocked(key) && (m_data.lexicon.find(key) >= 0 || m_user.isKnown(key));
}

bool WordScorer::isValid(const QString &word) const
{
    return isKnown(TextAnalysis::normalizeKey(word)) || m_data.spell(word);
}
