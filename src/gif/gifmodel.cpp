#include "gifmodel.h"

#include <algorithm>
#include <cmath>

GifModel::GifModel(QObject *parent)
    : QAbstractListModel(parent)
{ }

int GifModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : int(m_items.size());
}

QVariant GifModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid | CheckIndexOption::ParentIsInvalid)) {
        return {};
    }
    const Klipy::Item &item = m_items.at(index.row());
    const Tile tile = index.row() < m_tiles.size() ? m_tiles.at(index.row()) : Tile {};
    switch (role) {
    case IdRole:
        return item.id;
    case SlugRole:
        return item.slug;
    case TitleRole:
    case Qt::DisplayRole:
        return item.title;
    case TypeRole:
        return item.type;
    case PreviewUrlRole:
        return item.preview.url;
    case PreviewWidthRole:
        return item.preview.width;
    case PreviewHeightRole:
        return item.preview.height;
    case BlurPreviewRole:
        return item.blurPreview;
    case GifUrlRole:
        return item.linkGif.url;
    case FavoriteRole:
        return m_favorites && m_favorites->contains(item.id);
    case TileXRole:
        return tile.x;
    case TileYRole:
        return tile.y;
    case TileWidthRole:
        return tile.width;
    case TileHeightRole:
        return tile.height;
    default:
        return {};
    }
}

QHash<int, QByteArray> GifModel::roleNames() const
{
    return {
        {IdRole, "itemId"},
        {SlugRole, "slug"},
        {TitleRole, "title"},
        {TypeRole, "type"},
        {PreviewUrlRole, "previewUrl"},
        {PreviewWidthRole, "previewWidth"},
        {PreviewHeightRole, "previewHeight"},
        {BlurPreviewRole, "blurPreview"},
        {GifUrlRole, "gifUrl"},
        {FavoriteRole, "favorite"},
        {TileXRole, "tileX"},
        {TileYRole, "tileY"},
        {TileWidthRole, "tileWidth"},
        {TileHeightRole, "tileHeight"},
    };
}

const QList<Klipy::Item> &GifModel::items() const
{
    return m_items;
}

const Klipy::Item *GifModel::find(const QString &id) const
{
    const int row = indexOf(id);
    return row < 0 ? nullptr : &m_items.at(row);
}

int GifModel::indexOf(const QString &id) const
{
    for (int row = 0; row < m_items.size(); ++row) {
        if (m_items.at(row).id == id) {
            return row;
        }
    }
    return -1;
}

void GifModel::setItems(const QList<Klipy::Item> &items)
{
    beginResetModel();
    m_items = items;
    m_columnBottoms.clear();
    relayout(0);
    endResetModel();
    Q_EMIT countChanged();
}

void GifModel::appendItems(const QList<Klipy::Item> &items)
{
    QList<Klipy::Item> fresh;
    for (const Klipy::Item &item : items) {
        if (indexOf(item.id) < 0) {
            fresh.append(item);
        }
    }
    if (fresh.isEmpty()) {
        return;
    }
    const int first = int(m_items.size());
    beginInsertRows({}, first, first + int(fresh.size()) - 1);
    m_items.append(fresh);
    relayout(first);
    endInsertRows();
    Q_EMIT countChanged();
}

void GifModel::prepend(const Klipy::Item &item, int limit)
{
    QList<Klipy::Item> items = m_items;
    items.removeIf([&item](const Klipy::Item &existing) { return existing.id == item.id; });
    items.prepend(item);
    while (items.size() > limit) {
        items.removeLast();
    }
    setItems(items);
}

void GifModel::remove(const QString &id)
{
    const int row = indexOf(id);
    if (row < 0) {
        return;
    }
    beginRemoveRows({}, row, row);
    m_items.removeAt(row);
    endRemoveRows();
    relayoutAll();
    Q_EMIT countChanged();
}

void GifModel::clear()
{
    setItems({});
    setHasMore(false);
}

bool GifModel::hasMore() const
{
    return m_hasMore;
}

void GifModel::setHasMore(bool hasMore)
{
    if (m_hasMore != hasMore) {
        m_hasMore = hasMore;
        Q_EMIT hasMoreChanged();
    }
}

void GifModel::setFavoriteIds(const QSet<QString> *favorites)
{
    m_favorites = favorites;
}

void GifModel::favoriteChanged(const QString &id)
{
    const int row = indexOf(id);
    if (row >= 0) {
        Q_EMIT dataChanged(index(row), index(row), {FavoriteRole});
    }
}

int GifModel::columns() const
{
    return m_columns;
}

void GifModel::setColumns(int columns)
{
    updateGeometry(&m_columns, std::max(1, columns));
}

qreal GifModel::layoutWidth() const
{
    return m_layoutWidth;
}

void GifModel::setLayoutWidth(qreal width)
{
    updateGeometry(&m_layoutWidth, width);
}

qreal GifModel::spacing() const
{
    return m_spacing;
}

void GifModel::setSpacing(qreal spacing)
{
    updateGeometry(&m_spacing, spacing);
}

void GifModel::relayoutAll()
{
    m_columnBottoms.clear();
    relayout(0);
    if (!m_items.isEmpty()) {
        Q_EMIT dataChanged(index(0), index(int(m_items.size()) - 1), {TileXRole, TileYRole, TileWidthRole, TileHeightRole});
    }
}

qreal GifModel::contentHeight() const
{
    return m_contentHeight;
}

QVariantMap GifModel::get(int row) const
{
    QVariantMap map;
    if (row < 0 || row >= m_items.size()) {
        return map;
    }
    const QHash<int, QByteArray> roles = roleNames();
    for (auto it = roles.begin(); it != roles.end(); ++it) {
        map.insert(QString::fromLatin1(it.value()), data(index(row), it.key()));
    }
    return map;
}

void GifModel::relayout(int fromRow)
{
    if (fromRow == 0 || m_columnBottoms.size() != m_columns) {
        fromRow = 0;
        m_tiles.clear();
        m_columnBottoms = QList<qreal>(m_columns, 0.0);
    }
    const qreal columnWidth = std::max<qreal>(0.0, (m_layoutWidth - m_spacing * (m_columns - 1)) / m_columns);
    m_tiles.resize(fromRow);
    for (int row = fromRow; row < m_items.size(); ++row) {
        const Klipy::Item &item = m_items.at(row);
        const qreal aspect = item.preview.width > 0 && item.preview.height > 0 ? qreal(item.preview.height) / item.preview.width : 1.0;
        const auto shortest = std::min_element(m_columnBottoms.begin(), m_columnBottoms.end());
        const int column = int(std::distance(m_columnBottoms.begin(), shortest));
        Tile tile;
        tile.x = column * (columnWidth + m_spacing);
        tile.y = *shortest;
        tile.width = columnWidth;
        tile.height = std::round(columnWidth * std::clamp<qreal>(aspect, 0.45, 2.2));
        *shortest += tile.height + m_spacing;
        m_tiles.append(tile);
    }
    const qreal height
        = m_items.isEmpty() ? 0.0 : std::max<qreal>(0.0, *std::max_element(m_columnBottoms.begin(), m_columnBottoms.end()) - m_spacing);
    if (!qFuzzyCompare(m_contentHeight + 1.0, height + 1.0)) {
        m_contentHeight = height;
        Q_EMIT contentHeightChanged();
    }
}
