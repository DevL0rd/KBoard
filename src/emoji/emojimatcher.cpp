#include "emojimatcher.h"

#include <QRegularExpression>

#include <algorithm>
#include <cmath>

using namespace Qt::Literals::StringLiterals;

namespace
{
constexpr int flagsGroup = 8;
constexpr double strongScore = 100;
constexpr double weakScore = 50;

bool anyStartsWith(const QStringList &words, const QString &token)
{
    return std::ranges::any_of(words, [&token](const QString &word) { return word.startsWith(token); });
}

QString normalizedWord(const QString &word)
{
    static const QRegularExpression edges(u"^[^\\p{L}\\p{N}]+|[^\\p{L}\\p{N}]+$"_s);
    QString lower = word.toLower();
    lower.remove(edges);
    return lower;
}

bool isStopWord(const QString &word)
{
    static const QStringList stopWords = {u"an"_s, u"and"_s, u"at"_s, u"button"_s, u"for"_s, u"in"_s, u"is"_s, u"it"_s, u"of"_s, u"on"_s,
        u"or"_s, u"the"_s, u"to"_s, u"with"_s, u"up"_s, u"down"_s, u"me"_s, u"my"_s, u"no"_s, u"not"_s, u"new"_s, u"old"_s, u"one"_s,
        u"so"_s, u"sign"_s, u"symbol"_s, u"mark"_s, u"face"_s};
    return word.size() < 2 || stopWords.contains(word);
}
}

EmojiMatcher::EmojiMatcher(const EmojiData &data, QHash<int, int> usage)
    : m_data(data)
    , m_usage(std::move(usage))
{ }

int EmojiMatcher::usage(int index) const
{
    return m_usage.value(index, 0);
}

QList<int> EmojiMatcher::ranked(QList<Match> matches, int limit)
{
    std::ranges::stable_sort(matches, [](const Match &a, const Match &b) { return a.score > b.score; });
    QList<int> result;
    for (const Match &match : std::as_const(matches)) {
        if (limit > 0 && result.size() >= limit) {
            break;
        }
        result.append(match.index);
    }
    return result;
}

double EmojiMatcher::tokenScore(const EmojiEntry &entry, const QString &token) const
{
    if (entry.nameWords.contains(token)) {
        return 120;
    }
    if (entry.keywords.contains(token)) {
        return 100;
    }
    if (anyStartsWith(entry.nameWords, token)) {
        return 70;
    }
    if (anyStartsWith(entry.keywords, token)) {
        return 55;
    }
    if (anyStartsWith(m_data.subgroupWords(entry.subgroup), token)) {
        return 20;
    }
    return token.size() >= 3 && entry.lowerName.contains(token) ? 10 : 0;
}

double EmojiMatcher::searchScore(const EmojiEntry &entry, int index, const QString &query, const QStringList &tokens) const
{
    double score = 0;
    for (const QString &token : tokens) {
        const double part = tokenScore(entry, token);
        if (part <= 0) {
            return -1;
        }
        score += part;
    }
    if (entry.lowerName == query) {
        score += 1000;
    } else if (entry.lowerName.startsWith(query)) {
        score += 30;
    }
    if (entry.popularity >= 0) {
        score += std::max(4.0, 45.0 - entry.popularity * 0.25);
    }
    return score + std::min(30, usage(index) * 5) - entry.nameWords.size() * 0.5;
}

QList<EmojiMatcher::Match> EmojiMatcher::collect(const std::function<double(const EmojiEntry &, int)> &score) const
{
    QList<Match> matches;
    const QList<EmojiEntry> &entries = m_data.entries();
    for (qsizetype i = 0; i < entries.size(); ++i) {
        const double value = entries.at(i).supported ? score(entries.at(i), int(i)) : -1;
        if (value >= 0) {
            matches.append({int(i), value});
        }
    }
    return matches;
}

QList<int> EmojiMatcher::search(const QString &query, int limit) const
{
    const QString lowerQuery = query.trimmed().toLower();
    const QStringList tokens = emojiWords(lowerQuery);
    if (tokens.isEmpty()) {
        return {};
    }
    QList<Match> matches = collect(
        [this, &lowerQuery, &tokens](const EmojiEntry &entry, int index) { return searchScore(entry, index, lowerQuery, tokens); });
    const bool hasStrongMatch = std::ranges::any_of(matches, [](const Match &match) { return match.score >= strongScore; });
    if (hasStrongMatch) {
        matches.removeIf([](const Match &match) { return match.score < weakScore; });
    }
    return ranked(matches, limit);
}

QList<int> EmojiMatcher::searchKaomoji(const QString &query, int limit) const
{
    const QStringList tokens = emojiWords(query);
    if (tokens.isEmpty()) {
        return {};
    }
    QList<Match> matches;
    const QList<KaomojiEntry> &kaomoji = m_data.kaomoji();
    for (qsizetype i = 0; i < kaomoji.size(); ++i) {
        const QStringList &keywords = kaomoji.at(i).keywords;
        double score = -double(keywords.size());
        const bool all = std::ranges::all_of(tokens, [&keywords, &score](const QString &token) {
            const double part = keywords.contains(token) ? 100 : (anyStartsWith(keywords, token) ? 50 : 0);
            score += part;
            return part > 0;
        });
        if (all) {
            matches.append({int(i), score});
        }
    }
    return ranked(matches, limit);
}

double EmojiMatcher::suggestionScore(const EmojiEntry &entry, int index, const QString &token) const
{
    double score = 0;
    if (entry.lowerName == token) {
        score = 300;
    } else if (entry.keywords.contains(token)) {
        score = 100;
    } else if (entry.group != flagsGroup && entry.nameWords.contains(token)) {
        score = 90;
    } else {
        return -1;
    }
    if (entry.popularity >= 0) {
        score += 8.0 + 110.0 * std::exp(-entry.popularity / 12.0);
    } else {
        score -= entry.nameWords.size() * 2.0;
    }
    return score + std::min(40, usage(index) * 8);
}

QList<EmojiMatcher::Match> EmojiMatcher::suggestionMatches(const QString &token) const
{
    return collect([this, &token](const EmojiEntry &entry, int index) { return suggestionScore(entry, index, token); });
}

QList<int> EmojiMatcher::suggest(const QString &word, int limit) const
{
    const QString token = normalizedWord(word);
    if (isStopWord(token)) {
        return {};
    }
    QList<Match> matches = suggestionMatches(token);
    if (matches.isEmpty() && token.size() > 3 && token.endsWith(u's')) {
        matches = suggestionMatches(token.chopped(1));
    }
    return ranked(matches, limit);
}
