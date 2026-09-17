#pragma once

#include "keygeometry.h"

#include <QString>
#include <QVector>

#include <array>

class Lexicon;

class Corrector
{
public:
    struct Match
    {
        int entry = -1;
        double cost = 0;
    };

    explicit Corrector(const KeyGeometry &geometry);

    QVector<Match> search(const Lexicon &lexicon, const QString &typedKey, double maxCost) const;
    double distance(const QString &typedKey, const QString &candidateKey, double maxCost) const;

    double substitutionCost(QChar intended, QChar typed) const;
    double extraCost(const QString &typed, int index) const;

private:
    class Query;
    struct Row;

    double keyDistance(QChar a, QChar b) const;
    static double fillRow(const Row &row);

    static constexpr int TableSize = 26;

    KeyGeometry m_geometry;
    std::array<double, std::size_t(TableSize) * TableSize> m_distances {};
};
