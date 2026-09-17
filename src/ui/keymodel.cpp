#include "keymodel.h"

KeyModel::KeyModel(QObject *parent)
    : QAbstractListModel(parent)
{ }

int KeyModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : int(m_entries.size());
}

QVariant KeyModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_entries.size()) {
        return {};
    }
    const KeyEntry &entry = m_entries.at(index.row());
    switch (role) {
    case KeyRole:
        return entry.key;
    case TypeRole:
        return entry.key.value(QStringLiteral("type"));
    case LabelRole:
        return entry.key.value(QStringLiteral("label"));
    case XRole:
        return entry.rect.x();
    case YRole:
        return entry.rect.y();
    case WidthRole:
        return entry.rect.width();
    case HeightRole:
        return entry.rect.height();
    case RowRole:
        return entry.row;
    case ColumnRole:
        return entry.column;
    case HalfRole:
        return entry.half;
    case PressedRole:
        return entry.pressed;
    default:
        return {};
    }
}

QHash<int, QByteArray> KeyModel::roleNames() const
{
    return {
        {KeyRole, "keyData"},
        {TypeRole, "keyType"},
        {LabelRole, "label"},
        {XRole, "keyX"},
        {YRole, "keyY"},
        {WidthRole, "keyWidth"},
        {HeightRole, "keyHeight"},
        {RowRole, "row"},
        {ColumnRole, "column"},
        {HalfRole, "half"},
        {PressedRole, "pressed"},
    };
}

void KeyModel::reset(const QList<KeyEntry> &entries)
{
    beginResetModel();
    m_entries = entries;
    endResetModel();
}

void KeyModel::updateGeometry(const QList<KeyEntry> &entries)
{
    if (entries.size() != m_entries.size()) {
        reset(entries);
        return;
    }
    for (qsizetype i = 0; i < entries.size(); ++i) {
        m_entries[i].rect = entries.at(i).rect;
        m_entries[i].cell = entries.at(i).cell;
        m_entries[i].half = entries.at(i).half;
    }
    if (!m_entries.isEmpty()) {
        Q_EMIT dataChanged(index(0), index(int(m_entries.size()) - 1), {XRole, YRole, WidthRole, HeightRole, HalfRole});
    }
}

void KeyModel::setPressed(int row, bool pressed)
{
    if (row < 0 || row >= m_entries.size() || m_entries.at(row).pressed == pressed) {
        return;
    }
    m_entries[row].pressed = pressed;
    Q_EMIT dataChanged(index(row), index(row), {PressedRole});
}

const QList<KeyEntry> &KeyModel::entries() const
{
    return m_entries;
}
