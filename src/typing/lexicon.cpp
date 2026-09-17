#include "lexicon.h"

#include <QFile>

#include <QStringView>
#include <QVarLengthArray>
#include <functional>
#include <optional>

#include <algorithm>
#include <cmath>
#include <numeric>

namespace
{
std::optional<QByteArray> readDataFile(const QString &path, QByteArrayView magic, QString *error)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        *error = QStringLiteral("Cannot open %1: %2").arg(path, file.errorString());
        return std::nullopt;
    }
    QByteArray data = file.readAll();
    if (!data.startsWith(magic)) {
        *error = QStringLiteral("%1 has an unknown format").arg(path);
        return std::nullopt;
    }
    return data;
}

void forEachSeparated(QByteArrayView data, char separator, const std::function<void(QByteArrayView)> &callback)
{
    qsizetype position = 0;
    while (position < data.size()) {
        qsizetype end = data.indexOf(separator, position);
        if (end < 0) {
            end = data.size();
        }
        callback(data.sliced(position, end - position));
        position = end + 1;
    }
}

void forEachLine(const QByteArray &data, const std::function<void(QByteArrayView)> &callback)
{
    const qsizetype bodyStart = data.indexOf('\n') + 1;
    if (bodyStart > 0) {
        forEachSeparated(QByteArrayView(data).sliced(bodyStart), '\n', callback);
    }
}

void forEachField(QByteArrayView line, const std::function<void(QByteArrayView)> &callback)
{
    forEachSeparated(line, '\t', callback);
}
}

void WordTrie::build(const QVector<QString> &sortedKeys, const QVector<int> &entryForSorted)
{
    m_nodes.clear();
    m_edgeChars.clear();
    m_edgeTargets.clear();
    m_maxDepth = 0;
    m_nodes.reserve(sortedKeys.size() * 3);
    m_edgeChars.reserve(sortedKeys.size() * 3);
    m_edgeTargets.reserve(sortedKeys.size() * 3);
    buildRange(sortedKeys, entryForSorted, 0, sortedKeys.size(), 0);
}

int WordTrie::buildRange(const QVector<QString> &keys, const QVector<int> &entries, int low, int high, int depth)
{
    const int nodeIndex = m_nodes.size();
    m_nodes.append(Node {});
    m_maxDepth = std::max(m_maxDepth, depth);
    int cursor = low;
    if (cursor < high && keys[cursor].size() == depth) {
        m_nodes[nodeIndex].entry = entries[cursor];
        ++cursor;
    }
    struct Group
    {
        char16_t character;
        int low;
        int high;
    };
    QVarLengthArray<Group, 32> groups;
    while (cursor < high) {
        const char16_t character = keys[cursor].at(depth).unicode();
        int end = cursor + 1;
        while (end < high && keys[end].at(depth).unicode() == character) {
            ++end;
        }
        groups.append(Group {character, cursor, end});
        cursor = end;
    }
    const int firstEdge = m_edgeChars.size();
    m_nodes[nodeIndex].firstChild = firstEdge;
    m_nodes[nodeIndex].childCount = groups.size();
    m_edgeChars.resize(firstEdge + groups.size());
    m_edgeTargets.resize(firstEdge + groups.size());
    for (int i = 0; i < groups.size(); ++i) {
        m_edgeChars[firstEdge + i] = groups[i].character;
        const int child = buildRange(keys, entries, groups[i].low, groups[i].high, depth + 1);
        m_edgeTargets[firstEdge + i] = child;
    }
    return nodeIndex;
}

bool Lexicon::load(const QString &path, QString *error)
{
    const std::optional<QByteArray> data = readDataFile(path, "#kboard-freq 1", error);
    if (!data) {
        return false;
    }
    m_entries.clear();
    m_byKey.clear();
    forEachLine(*data, [this](QByteArrayView line) { appendEntry(line); });
    if (m_entries.isEmpty()) {
        *error = QStringLiteral("Word list %1 is empty").arg(path);
        return false;
    }
    buildIndexes();
    return true;
}

void Lexicon::appendEntry(QByteArrayView line)
{
    const qsizetype tab = line.indexOf('\t');
    if (tab <= 0) {
        return;
    }
    LexiconEntry entry;
    entry.form = QString::fromUtf8(line.first(tab));
    entry.key = entry.form.toLower();
    bool ok = false;
    entry.count = line.sliced(tab + 1).toUInt(&ok);
    if (!ok || m_byKey.contains(entry.key)) {
        return;
    }
    entry.logFrequency = float(std::log(double(entry.count) + 1.0));
    m_byKey.insert(entry.key, int(m_entries.size()));
    m_entries.append(std::move(entry));
}

void Lexicon::buildIndexes()
{
    QVector<int> order(m_entries.size());
    std::iota(order.begin(), order.end(), 0);
    std::sort(order.begin(), order.end(), [this](int a, int b) { return m_entries[a].key < m_entries[b].key; });
    m_sortedKeys.clear();
    m_sortedKeys.reserve(order.size());
    for (int index : std::as_const(order)) {
        m_sortedKeys.append(m_entries[index].key);
    }
    m_sortedEntries = order;
    m_trie.build(m_sortedKeys, m_sortedEntries);
}

int Lexicon::find(const QString &key) const
{
    return m_byKey.value(key, -1);
}

QVector<int> Lexicon::prefixMatches(const QString &prefix, int limit, int minExtraLength) const
{
    QVector<int> best;
    if (prefix.isEmpty() || limit <= 0) {
        return best;
    }
    auto it = std::lower_bound(m_sortedKeys.cbegin(), m_sortedKeys.cend(), prefix);
    const int minLength = prefix.size() + minExtraLength;
    for (; it != m_sortedKeys.cend() && it->startsWith(prefix); ++it) {
        if (it->size() < minLength) {
            continue;
        }
        const int entryIndex = m_sortedEntries[int(it - m_sortedKeys.cbegin())];
        const float frequency = m_entries[entryIndex].logFrequency;
        if (best.size() == limit && m_entries[best.last()].logFrequency >= frequency) {
            continue;
        }
        auto position = std::upper_bound(
            best.begin(), best.end(), frequency, [this](float value, int candidate) { return value > m_entries[candidate].logFrequency; });
        best.insert(position, entryIndex);
        if (best.size() > limit) {
            best.removeLast();
        }
    }
    return best;
}

bool NGramModel::load(const QString &path, const Lexicon &lexicon, QString *error)
{
    const std::optional<QByteArray> data = readDataFile(path, "#kboard-ngram 1", error);
    if (!data) {
        return false;
    }
    m_followers.clear();
    forEachLine(*data, [this, &lexicon](QByteArrayView line) { appendContext(line, lexicon); });
    return true;
}

void NGramModel::appendContext(QByteArrayView line, const Lexicon &lexicon)
{
    const qsizetype tab = line.indexOf('\t');
    if (tab <= 0) {
        return;
    }
    QVector<Follower> followers;
    double total = 0;
    forEachField(line.sliced(tab + 1), [&](QByteArrayView field) {
        const qsizetype space = field.lastIndexOf(' ');
        const int entry = space > 0 ? lexicon.find(QString::fromUtf8(field.first(space))) : -1;
        const double count = space > 0 ? field.sliced(space + 1).toDouble() : 0.0;
        if (entry >= 0 && count > 0) {
            followers.append(Follower {entry, float(count)});
            total += count;
        }
    });
    for (Follower &follower : followers) {
        follower.probability = float(follower.probability / total);
    }
    if (!followers.isEmpty()) {
        m_followers.insert(QString::fromUtf8(line.first(tab)), std::move(followers));
    }
}

const QVector<NGramModel::Follower> *NGramModel::followers(const QString &context) const
{
    auto it = m_followers.constFind(context);
    return it == m_followers.cend() ? nullptr : &it.value();
}
