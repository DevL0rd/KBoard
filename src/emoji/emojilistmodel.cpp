#include "emojilistmodel.h"
#include "emojistore.h"

EmojiListModel::EmojiListModel(EmojiStore *store, Display display, QObject *parent)
    : QAbstractListModel(parent)
    , m_store(store)
    , m_display(display)
{
    connect(store, &EmojiStore::tonesChanged, this, &EmojiListModel::refreshAll);
    connect(store, &EmojiStore::favoritesChanged, this, &EmojiListModel::refreshAll);
}

int EmojiListModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : int(m_emoji.size());
}

QVariant EmojiListModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid | CheckIndexOption::ParentIsInvalid)) {
        return {};
    }
    const QString &value = m_emoji.at(index.row());
    switch (role) {
    case Qt::DisplayRole:
    case EmojiRole:
        return m_display == Display::PreferredTone ? m_store->displayEmoji(value) : value;
    case BaseEmojiRole:
        return m_store->baseEmoji(value);
    case NameRole:
        return m_store->nameOf(value);
    case HasSkinTonesRole:
        return m_store->hasSkinTones(value);
    case FavoriteRole:
        return m_store->isFavorite(value);
    case GroupRole: {
        const int entry = m_store->data().indexOf(value);
        return entry < 0 ? QString() : m_store->data().groups().at(m_store->data().entries().at(entry).group).id;
    }
    default:
        return {};
    }
}

QHash<int, QByteArray> EmojiListModel::roleNames() const
{
    return {
        {EmojiRole, "emoji"},
        {BaseEmojiRole, "baseEmoji"},
        {NameRole, "name"},
        {HasSkinTonesRole, "hasSkinTones"},
        {FavoriteRole, "favorite"},
        {GroupRole, "group"},
    };
}

void EmojiListModel::setEmoji(const QStringList &emoji)
{
    if (emoji == m_emoji) {
        return;
    }
    const bool countChanges = emoji.size() != m_emoji.size();
    beginResetModel();
    m_emoji = emoji;
    endResetModel();
    if (countChanges) {
        Q_EMIT countChanged();
    }
}

QString EmojiListModel::get(int row) const
{
    return m_emoji.value(row);
}

void EmojiListModel::refreshAll()
{
    if (!m_emoji.isEmpty()) {
        Q_EMIT dataChanged(index(0), index(int(m_emoji.size()) - 1));
    }
}
