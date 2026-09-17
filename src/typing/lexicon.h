#pragma once

#include <QByteArrayView>
#include <QHash>
#include <QString>
#include <QVector>

struct LexiconEntry
{
    QString form;
    QString key;
    quint32 count = 0;
    float logFrequency = 0;
};

class WordTrie
{
public:
    struct Node
    {
        int firstChild = 0;
        int childCount = 0;
        int entry = -1;
    };

    void build(const QVector<QString> &sortedKeys, const QVector<int> &entryForSorted);
    const Node &node(int index) const { return m_nodes[index]; }
    char16_t edgeChar(int edge) const { return m_edgeChars[edge]; }
    int edgeTarget(int edge) const { return m_edgeTargets[edge]; }
    int maxDepth() const { return m_maxDepth; }
    bool isEmpty() const { return m_nodes.isEmpty(); }

private:
    int buildRange(const QVector<QString> &keys, const QVector<int> &entries, int low, int high, int depth);

    QVector<Node> m_nodes;
    QVector<char16_t> m_edgeChars;
    QVector<int> m_edgeTargets;
    int m_maxDepth = 0;
};

class Lexicon
{
public:
    bool load(const QString &path, QString *error);

    int size() const { return m_entries.size(); }
    const LexiconEntry &entry(int index) const { return m_entries[index]; }
    int find(const QString &key) const;
    QVector<int> prefixMatches(const QString &prefix, int limit, int minExtraLength) const;
    const WordTrie &trie() const { return m_trie; }

private:
    void appendEntry(QByteArrayView line);
    void buildIndexes();

    QVector<LexiconEntry> m_entries;
    QHash<QString, int> m_byKey;
    QVector<QString> m_sortedKeys;
    QVector<int> m_sortedEntries;
    WordTrie m_trie;
};

class NGramModel
{
public:
    struct Follower
    {
        int entry = -1;
        float probability = 0;
    };

    bool load(const QString &path, const Lexicon &lexicon, QString *error);
    const QVector<Follower> *followers(const QString &context) const;
    bool isEmpty() const { return m_followers.isEmpty(); }

private:
    void appendContext(QByteArrayView line, const Lexicon &lexicon);

    QHash<QString, QVector<Follower>> m_followers;
};
