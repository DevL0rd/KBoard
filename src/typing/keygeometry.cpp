#include "keygeometry.h"

#include <QVariantMap>

#include <algorithm>
#include <cmath>

KeyGeometry KeyGeometry::fromVariantList(const QVariantList &keys)
{
    KeyGeometry geometry;
    for (const QVariant &value : keys) {
        const QVariantMap map = value.toMap();
        const QString label = map.value(QStringLiteral("label")).toString();
        if (label.size() != 1 || !label.at(0).isLetter()) {
            continue;
        }
        const QChar letter = label.at(0).toLower();
        if (geometry.m_index.contains(letter)) {
            continue;
        }
        const double width = map.value(QStringLiteral("width")).toDouble();
        const double height = map.value(QStringLiteral("height")).toDouble();
        if (width <= 0 || height <= 0) {
            continue;
        }
        Key key;
        key.letter = letter;
        key.width = width;
        key.height = height;
        key.centre
            = QPointF(map.value(QStringLiteral("x")).toDouble() + width / 2.0, map.value(QStringLiteral("y")).toDouble() + height / 2.0);
        geometry.m_index.insert(letter, geometry.m_keys.size());
        geometry.m_keys.append(key);
    }
    geometry.finish();
    return geometry;
}

KeyGeometry KeyGeometry::qwerty()
{
    const QStringList rows = {QStringLiteral("qwertyuiop"), QStringLiteral("asdfghjkl"), QStringLiteral("zxcvbnm")};
    const double offsets[] = {0.0, 0.5, 1.5};
    QVariantList keys;
    for (int row = 0; row < rows.size(); ++row) {
        for (int column = 0; column < rows[row].size(); ++column) {
            keys.append(QVariantMap {
                {QStringLiteral("label"), QString(rows[row].at(column))},
                {QStringLiteral("x"), offsets[row] + column},
                {QStringLiteral("y"), double(row) * 1.3},
                {QStringLiteral("width"), 1.0},
                {QStringLiteral("height"), 1.3},
            });
        }
    }
    return fromVariantList(keys);
}

void KeyGeometry::finish()
{
    if (m_keys.isEmpty()) {
        m_unit = 1.0;
        m_fingerprint = 0;
        return;
    }
    QVector<double> widths;
    widths.reserve(m_keys.size());
    for (const Key &key : std::as_const(m_keys)) {
        widths.append(key.width);
    }
    std::sort(widths.begin(), widths.end());
    m_unit = widths.at(widths.size() / 2);
    quint64 hash = 1469598103934665603ULL;
    auto mix = [&hash](quint64 value) {
        hash ^= value;
        hash *= 1099511628211ULL;
    };
    for (const Key &key : std::as_const(m_keys)) {
        mix(key.letter.unicode());
        mix(quint64(std::llround(key.centre.x() / m_unit * 1000.0)));
        mix(quint64(std::llround(key.centre.y() / m_unit * 1000.0)));
    }
    m_fingerprint = hash;
}

bool KeyGeometry::isEmpty() const
{
    return m_keys.isEmpty();
}

int KeyGeometry::keyCount() const
{
    return m_keys.size();
}

int KeyGeometry::indexOf(QChar letter) const
{
    return m_index.value(letter, -1);
}

double KeyGeometry::unit() const
{
    return m_unit;
}

double KeyGeometry::distance(QChar a, QChar b) const
{
    const int ia = indexOf(a);
    const int ib = indexOf(b);
    if (ia < 0 || ib < 0) {
        return 10.0;
    }
    const QPointF delta = m_keys.at(ia).centre - m_keys.at(ib).centre;
    return std::hypot(delta.x(), delta.y()) / m_unit;
}

QPointF KeyGeometry::normalizedCentre(int index) const
{
    return m_keys.at(index).centre / m_unit;
}

quint64 KeyGeometry::fingerprint() const
{
    return m_fingerprint;
}
