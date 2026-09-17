#pragma once

#include "keyentry.h"

#include <QAbstractListModel>

class KeyModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles
    {
        KeyRole = Qt::UserRole + 1,
        TypeRole,
        LabelRole,
        XRole,
        YRole,
        WidthRole,
        HeightRole,
        RowRole,
        ColumnRole,
        HalfRole,
        PressedRole,
    };

    explicit KeyModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void reset(const QList<KeyEntry> &entries);
    void updateGeometry(const QList<KeyEntry> &entries);
    void setPressed(int row, bool pressed);
    const QList<KeyEntry> &entries() const;

private:
    QList<KeyEntry> m_entries;
};
