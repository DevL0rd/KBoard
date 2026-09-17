#include "keylayout.h"

#include <cmath>
#include <limits>

namespace
{
qreal widthOf(const QVariant &key)
{
    return key.toMap().value(QStringLiteral("width")).toReal();
}

qreal unitsOf(const QVariantList &keys)
{
    qreal total = 0;
    for (const QVariant &key : keys) {
        total += widthOf(key);
    }
    return total;
}

struct Piece
{
    QVariantMap key;
    qreal units = 0;
};

struct Row
{
    QList<Piece> left;
    QList<Piece> right;
    qreal weight = 1;
};

struct Band
{
    qreal start = 0;
    qreal width = 0;
    qreal unit = 0;
    qreal y = 0;
    qreal height = 0;
    int row = 0;
    int half = -1;
};

qreal unitsOf(const QList<Piece> &pieces)
{
    qreal total = 0;
    for (const Piece &piece : pieces) {
        total += piece.units;
    }
    return total;
}

Row splitRow(const QVariantMap &data, bool split)
{
    Row row;
    row.weight = data.value(QStringLiteral("height"), 1.0).toReal();
    const QVariantList keys = data.value(QStringLiteral("keys")).toList();
    if (!split) {
        for (const QVariant &key : keys) {
            row.left.append(Piece {key.toMap(), widthOf(key)});
        }
        return row;
    }
    KeyLayout::SplitPoint point = KeyLayout::splitPoint(keys);
    const int forced = data.value(QStringLiteral("split"), -1).toInt();
    if (forced >= 0) {
        point = {qMin(forced, int(keys.size())), 0};
    }
    for (int i = 0; i < keys.size(); ++i) {
        const Piece piece {keys.at(i).toMap(), widthOf(keys.at(i))};
        if (i == point.index && point.leftPart > 0) {
            row.left.append(Piece {piece.key, point.leftPart});
            row.right.append(Piece {piece.key, piece.units - point.leftPart});
        } else {
            (i < point.index ? row.left : row.right).append(piece);
        }
    }
    return row;
}

void place(QList<KeyEntry> &entries, const QList<Piece> &pieces, const Band &band, qreal gap)
{
    const qreal units = unitsOf(pieces);
    qreal x = band.start + (band.width - units * band.unit) / 2;
    const qreal inset = gap / 2;
    for (int i = 0; i < pieces.size(); ++i) {
        const qreal width = pieces.at(i).units * band.unit;
        KeyEntry entry;
        entry.key = pieces.at(i).key;
        entry.row = band.row;
        entry.column = int(entries.size());
        entry.half = band.half;
        entry.rect = QRectF(x + inset, band.y + inset, qMax<qreal>(0, width - gap), qMax<qreal>(0, band.height - gap));
        const qreal cellStart = i == 0 ? band.start : x;
        const qreal cellEnd = i == pieces.size() - 1 ? band.start + band.width : x + width;
        entry.cell = QRectF(cellStart, band.y, cellEnd - cellStart, band.height);
        entries.append(entry);
        x += width;
    }
}

qreal halfColumns(const QList<Row> &rows, int columns)
{
    qreal result = columns / 2.0;
    for (const Row &row : rows) {
        if (row.weight >= 1.0) {
            result = qMax(result, qMax(unitsOf(row.left), unitsOf(row.right)));
        }
    }
    return result;
}

void renumberColumns(QList<KeyEntry> &entries)
{
    int previousRow = -1;
    int column = 0;
    for (KeyEntry &entry : entries) {
        column = entry.row == previousRow ? column + 1 : 0;
        previousRow = entry.row;
        entry.column = column;
    }
}
}

namespace KeyLayout
{
SplitPoint splitPoint(const QVariantList &keys)
{
    const qreal target = unitsOf(keys) / 2;
    qreal cumulative = 0;
    for (int i = 0; i < keys.size(); ++i) {
        const qreal width = widthOf(keys.at(i));
        if (std::abs(cumulative - target) < 1e-6) {
            return {i, 0};
        }
        if (cumulative + width > target) {
            const bool space = keys.at(i).toMap().value(QStringLiteral("type")).toString() == QLatin1String("space");
            if (space && target - cumulative >= 0.5 && cumulative + width - target >= 0.5) {
                return {i, target - cumulative};
            }
            return {(target - cumulative) < (cumulative + width - target) ? i : i + 1, 0};
        }
        cumulative += width;
    }
    return {int(keys.size()), 0};
}

qreal halfWidth(const Params &params)
{
    return params.split ? qMax<qreal>(0, (params.width - params.splitGap) / 2) : params.width;
}

QList<KeyEntry> compute(const QVariantMap &page, const Params &params)
{
    QList<KeyEntry> entries;
    const int columns = qMax(1, page.value(QStringLiteral("columns")).toInt());
    QList<Row> rows;
    qreal totalWeight = 0;
    for (const QVariant &data : page.value(QStringLiteral("rows")).toList()) {
        rows.append(splitRow(data.toMap(), params.split));
        totalWeight += rows.constLast().weight;
    }
    if (rows.isEmpty() || totalWeight <= 0) {
        return entries;
    }

    const qreal half = halfWidth(params);
    const qreal sharedColumns = halfColumns(rows, columns);
    qreal y = 0;
    for (int r = 0; r < rows.size(); ++r) {
        const Row &row = rows.at(r);
        const qreal height = params.height * row.weight / totalWeight;
        if (params.split) {
            const qreal rowColumns = row.weight >= 1.0 ? sharedColumns : qMax(columns / 2.0, qMax(unitsOf(row.left), unitsOf(row.right)));
            const qreal unit = half / rowColumns;
            place(entries, row.left, Band {0, half, unit, y, height, r, 0}, params.gap);
            place(entries, row.right, Band {params.width - half, half, unit, y, height, r, 1}, params.gap);
        } else {
            const qreal unit = params.width / qMax<qreal>(columns, unitsOf(row.left));
            place(entries, row.left, Band {0, params.width, unit, y, height, r, -1}, params.gap);
        }
        y += height;
    }
    renumberColumns(entries);
    return entries;
}

int keyAt(const QList<KeyEntry> &entries, QPointF point)
{
    for (int i = 0; i < entries.size(); ++i) {
        if (entries.at(i).cell.contains(point)) {
            return i;
        }
    }
    return -1;
}

int neighbor(const QList<KeyEntry> &entries, int index, int dx, int dy)
{
    if (entries.isEmpty()) {
        return -1;
    }
    if (index < 0 || index >= entries.size()) {
        return 0;
    }
    const KeyEntry &current = entries.at(index);
    if (dx != 0) {
        const int target = index + (dx > 0 ? 1 : -1);
        const bool sameRow = target >= 0 && target < entries.size() && entries.at(target).row == current.row;
        return sameRow ? target : index;
    }
    if (dy == 0) {
        return index;
    }
    const int row = current.row + (dy > 0 ? 1 : -1);
    const qreal center = current.rect.center().x();
    int best = index;
    qreal bestDistance = std::numeric_limits<qreal>::max();
    for (int i = 0; i < entries.size(); ++i) {
        const qreal distance = std::abs(entries.at(i).rect.center().x() - center);
        if (entries.at(i).row == row && distance < bestDistance) {
            bestDistance = distance;
            best = i;
        }
    }
    return best;
}

QVariantList halfRects(const Params &params)
{
    if (!params.split) {
        return {QRectF(0, 0, params.width, params.height)};
    }
    const qreal half = halfWidth(params);
    return {QRectF(0, 0, half, params.height), QRectF(params.width - half, 0, half, params.height)};
}
}
