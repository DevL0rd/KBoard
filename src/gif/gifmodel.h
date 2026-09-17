#pragma once

#include "klipyapi.h"

#include <QAbstractListModel>
#include <QQmlEngine>
#include <QSet>

class GifModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("GifModel instances are owned by GifStore")

    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
    Q_PROPERTY(bool hasMore READ hasMore NOTIFY hasMoreChanged)
    Q_PROPERTY(int columns READ columns WRITE setColumns NOTIFY geometryChanged)
    Q_PROPERTY(qreal layoutWidth READ layoutWidth WRITE setLayoutWidth NOTIFY geometryChanged)
    Q_PROPERTY(qreal spacing READ spacing WRITE setSpacing NOTIFY geometryChanged)
    Q_PROPERTY(qreal contentHeight READ contentHeight NOTIFY contentHeightChanged)

public:
    enum Role
    {
        IdRole = Qt::UserRole + 1,
        SlugRole,
        TitleRole,
        TypeRole,
        PreviewUrlRole,
        PreviewWidthRole,
        PreviewHeightRole,
        BlurPreviewRole,
        GifUrlRole,
        FavoriteRole,
        TileXRole,
        TileYRole,
        TileWidthRole,
        TileHeightRole,
    };
    Q_ENUM(Role)

    explicit GifModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    const QList<Klipy::Item> &items() const;
    const Klipy::Item *find(const QString &id) const;
    int indexOf(const QString &id) const;
    void setItems(const QList<Klipy::Item> &items);
    void appendItems(const QList<Klipy::Item> &items);
    void prepend(const Klipy::Item &item, int limit);
    void remove(const QString &id);
    void clear();

    bool hasMore() const;
    void setHasMore(bool hasMore);

    void setFavoriteIds(const QSet<QString> *favorites);
    void favoriteChanged(const QString &id);

    int columns() const;
    void setColumns(int columns);
    qreal layoutWidth() const;
    void setLayoutWidth(qreal width);
    qreal spacing() const;
    void setSpacing(qreal spacing);
    qreal contentHeight() const;

    Q_INVOKABLE QVariantMap get(int row) const;

Q_SIGNALS:
    void countChanged();
    void hasMoreChanged();
    void geometryChanged();
    void contentHeightChanged();

private:
    struct Tile
    {
        qreal x = 0;
        qreal y = 0;
        qreal width = 0;
        qreal height = 0;
    };

    void relayout(int fromRow);
    void relayoutAll();

    template<typename T> void updateGeometry(T *field, T value)
    {
        if (*field == value) {
            return;
        }
        *field = value;
        relayoutAll();
        Q_EMIT geometryChanged();
    }

    QList<Klipy::Item> m_items;
    QList<Tile> m_tiles;
    QList<qreal> m_columnBottoms;
    const QSet<QString> *m_favorites = nullptr;
    bool m_hasMore = false;
    int m_columns = 3;
    qreal m_layoutWidth = 0;
    qreal m_spacing = 4;
    qreal m_contentHeight = 0;
};
