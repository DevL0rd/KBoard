#include "corrector.h"

#include "lexicon.h"

#include <QHash>
#include <QVarLengthArray>

#include <algorithm>
#include <limits>

namespace
{
constexpr double TranspositionCost = 0.6;
constexpr double FirstLetterPenalty = 0.15;
constexpr double NeighbourReach = 1.6;
constexpr double SlipReach = 1.2;
constexpr int Letters = 26;
constexpr int ApostropheSlot = Letters;
constexpr int Slots = Letters + 1;

bool isVowel(QChar c)
{
    return QStringView(u"aeiouy").contains(c);
}

QChar baseLetter(QChar c)
{
    const QString decomposed = QString(c).normalized(QString::NormalizationForm_D);
    return decomposed.isEmpty() ? c : decomposed.at(0);
}

int letterIndex(QChar c)
{
    const int index = c.unicode() - u'a';
    return index >= 0 && index < Letters ? index : -1;
}

int slotIndex(QChar c)
{
    return c == u'\'' ? ApostropheSlot : letterIndex(c);
}

double missingCost(QChar intended, QChar previousIntended)
{
    if (intended == u'\'') {
        return 0.15;
    }
    if (intended == u'-') {
        return 0.3;
    }
    return intended == previousIntended ? 0.35 : 1.0;
}
}

class Corrector::Query
{
public:
    Query(const Corrector &corrector, const QString &typed)
        : m_corrector(corrector)
        , m_typed(typed)
        , m_extra(typed.size() + 1)
        , m_table(qsizetype(Slots) * typed.size())
    {
        for (int j = 0; j < typed.size(); ++j) {
            m_extra[j] = corrector.extraCost(typed, j);
        }
        for (int slot = 0; slot < Slots; ++slot) {
            const QChar intended = slot == ApostropheSlot ? QChar(u'\'') : QChar(u'a' + slot);
            fillSubstitutions(intended, m_table.data() + slot * typed.size());
        }
    }

    const QString &typed() const { return m_typed; }

    double extra(int j) const { return m_extra[j]; }

    const double *substitutions(QChar intended) const
    {
        const int slot = slotIndex(intended);
        if (slot >= 0) {
            return m_table.constData() + slot * m_typed.size();
        }
        auto it = m_other.find(intended.unicode());
        if (it == m_other.end()) {
            it = m_other.insert(intended.unicode(), QVector<double>(m_typed.size()));
            fillSubstitutions(intended, it->data());
        }
        return it->constData();
    }

private:
    void fillSubstitutions(QChar intended, double *out) const
    {
        for (int j = 0; j < m_typed.size(); ++j) {
            out[j] = m_corrector.substitutionCost(intended, m_typed.at(j));
        }
    }

    const Corrector &m_corrector;
    const QString &m_typed;
    QVarLengthArray<double, 32> m_extra;
    QVarLengthArray<double, 1024> m_table;
    mutable QHash<char16_t, QVector<double>> m_other;
};

struct Corrector::Row
{
    const Query &query;
    const double *above;
    const double *twoAbove;
    double *cells;
    QChar intended;
    QChar previous;
};

Corrector::Corrector(const KeyGeometry &geometry)
    : m_geometry(geometry)
{
    for (int a = 0; a < TableSize; ++a) {
        for (int b = 0; b < TableSize; ++b) {
            m_distances[a * TableSize + b] = m_geometry.distance(QChar(u'a' + a), QChar(u'a' + b));
        }
    }
}

double Corrector::keyDistance(QChar a, QChar b) const
{
    const int ia = letterIndex(a);
    const int ib = letterIndex(b);
    return ia >= 0 && ib >= 0 ? m_distances[ia * TableSize + ib] : m_geometry.distance(a, b);
}

double Corrector::substitutionCost(QChar intended, QChar typed) const
{
    if (intended == typed) {
        return 0;
    }
    if ((intended.unicode() > 127 || typed.unicode() > 127) && baseLetter(intended) == baseLetter(typed)) {
        return 0.2;
    }
    const double reach = keyDistance(intended, typed);
    if (reach <= NeighbourReach) {
        return 0.35 + 0.25 * reach;
    }
    return isVowel(intended) && isVowel(typed) ? 0.85 : 1.0;
}

double Corrector::extraCost(const QString &typed, int index) const
{
    const QChar c = typed.at(index);
    if (c == u'\'' || c == u'-') {
        return 0.3;
    }
    if (index > 0 && typed.at(index - 1) == c) {
        return 0.45;
    }
    const bool slipBefore = index > 0 && keyDistance(c, typed.at(index - 1)) <= SlipReach;
    const bool slipAfter = index + 1 < typed.size() && keyDistance(c, typed.at(index + 1)) <= SlipReach;
    return slipBefore || slipAfter ? 0.7 : 1.0;
}

double Corrector::fillRow(const Row &row)
{
    const QString &typed = row.query.typed();
    const int n = typed.size();
    const bool firstLetter = row.twoAbove == nullptr;
    const double missing = missingCost(row.intended, row.previous);
    const double *substitution = row.query.substitutions(row.intended);
    row.cells[0] = row.above[0] + missing + (firstLetter ? FirstLetterPenalty : 0.0);
    double reachable = row.cells[0];
    for (int j = 1; j <= n; ++j) {
        const double sub = substitution[j - 1];
        double value = row.above[j - 1] + sub + (sub > 0 && firstLetter && j == 1 ? FirstLetterPenalty : 0.0);
        value = std::min({value, row.above[j] + missing, row.cells[j - 1] + row.query.extra(j - 1)});
        if (!firstLetter && j >= 2 && row.intended != row.previous && row.intended == typed.at(j - 2) && row.previous == typed.at(j - 1)) {
            value = std::min(value, row.twoAbove[j - 2] + TranspositionCost);
        }
        if (j >= 2 && typed.at(j - 1) == row.intended) {
            reachable = std::min(reachable, row.above[j - 2] + TranspositionCost);
        }
        row.cells[j] = value;
        reachable = std::min(reachable, value);
    }
    return reachable;
}

QVector<Corrector::Match> Corrector::search(const Lexicon &lexicon, const QString &typedKey, double maxCost) const
{
    QVector<Match> matches;
    const WordTrie &trie = lexicon.trie();
    if (trie.isEmpty() || typedKey.isEmpty()) {
        return matches;
    }
    const Query query(*this, typedKey);
    const qsizetype width = typedKey.size() + 1;
    const qsizetype maxDepth = trie.maxDepth() + 1;
    QVarLengthArray<double, 1024> rows((maxDepth + 1) * width);
    QVarLengthArray<char16_t, 64> path(maxDepth + 1);
    path[0] = 0;
    rows[0] = 0;
    for (int j = 1; j < width; ++j) {
        rows[j] = rows[j - 1] + query.extra(j - 1);
    }
    struct Frame
    {
        int node;
        int nextEdge;
    };
    QVarLengthArray<Frame, 64> stack;
    stack.append(Frame {0, trie.node(0).firstChild});
    while (!stack.isEmpty()) {
        const Frame frame = stack.last();
        const WordTrie::Node &node = trie.node(frame.node);
        if (frame.nextEdge >= node.firstChild + node.childCount) {
            stack.removeLast();
            continue;
        }
        ++stack.last().nextEdge;
        const qsizetype depth = stack.size();
        path[depth] = trie.edgeChar(frame.nextEdge);
        const double *twoAbove = depth >= 2 ? rows.constData() + (depth - 2) * width : nullptr;
        const Row row {query, rows.constData() + (depth - 1) * width, twoAbove, rows.data() + depth * width, QChar(path[depth]),
            QChar(path[depth - 1])};
        const double reachable = fillRow(row);
        const int target = trie.edgeTarget(frame.nextEdge);
        const WordTrie::Node &child = trie.node(target);
        if (child.entry >= 0 && row.cells[typedKey.size()] <= maxCost) {
            matches.append(Match {child.entry, row.cells[typedKey.size()]});
        }
        if (reachable <= maxCost && child.childCount > 0 && depth < maxDepth) {
            stack.append(Frame {target, child.firstChild});
        }
    }
    return matches;
}

double Corrector::distance(const QString &typedKey, const QString &candidateKey, double maxCost) const
{
    const Query query(*this, typedKey);
    const qsizetype width = typedKey.size() + 1;
    QVarLengthArray<double, 1024> rows((candidateKey.size() + 1) * width);
    rows[0] = 0;
    for (int j = 1; j < width; ++j) {
        rows[j] = rows[j - 1] + query.extra(j - 1);
    }
    for (qsizetype i = 1; i <= candidateKey.size(); ++i) {
        const double *twoAbove = i >= 2 ? rows.constData() + (i - 2) * width : nullptr;
        const QChar previous = i >= 2 ? candidateKey.at(i - 2) : QChar();
        const Row row {query, rows.constData() + (i - 1) * width, twoAbove, rows.data() + i * width, candidateKey.at(i - 1), previous};
        if (fillRow(row) > maxCost) {
            return std::numeric_limits<double>::infinity();
        }
    }
    return rows[candidateKey.size() * width + typedKey.size()];
}
