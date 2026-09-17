#pragma once

#include "emojidata.h"

#include <QHash>
#include <QList>

#include <functional>

class EmojiMatcher
{
public:
    EmojiMatcher(const EmojiData &data, QHash<int, int> usage);

    QList<int> search(const QString &query, int limit) const;
    QList<int> searchKaomoji(const QString &query, int limit) const;
    QList<int> suggest(const QString &word, int limit) const;

private:
    struct Match
    {
        int index = -1;
        double score = 0;
    };

    static QList<int> ranked(QList<Match> matches, int limit);
    double tokenScore(const EmojiEntry &entry, const QString &token) const;
    QList<Match> collect(const std::function<double(const EmojiEntry &, int)> &score) const;
    double searchScore(const EmojiEntry &entry, int index, const QString &query, const QStringList &tokens) const;
    double suggestionScore(const EmojiEntry &entry, int index, const QString &token) const;
    QList<Match> suggestionMatches(const QString &token) const;
    int usage(int index) const;

    const EmojiData &m_data;
    QHash<int, int> m_usage;
};
