#include "emojigridmodel.h"
#include "emojistore.h"

using namespace Qt::Literals::StringLiterals;

namespace
{
constexpr int maxEmojiResults = 240;
constexpr int maxKaomojiResults = 60;
}

EmojiGridModel::EmojiGridModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_store(EmojiStore::instance())
{
    connect(m_store, &EmojiStore::recentsChanged, this, &EmojiGridModel::markStale);
    connect(m_store, &EmojiStore::favoritesChanged, this, &EmojiGridModel::markStale);
    connect(m_store, &EmojiStore::favoritesChanged, this, &EmojiGridModel::refreshCells);
    connect(m_store, &EmojiStore::tonesChanged, this, &EmojiGridModel::refreshCells);
    rebuild();
}

int EmojiGridModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : int(m_rows.size());
}

QVariant EmojiGridModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid | CheckIndexOption::ParentIsInvalid)) {
        return {};
    }
    static const QStringList kindNames = {u"header"_s, u"emoji"_s, u"kaomoji"_s, u"empty"_s};
    const Row &row = m_rows.at(index.row());
    switch (role) {
    case KindRole:
        return kindNames.at(int(row.kind));
    case TitleRole:
        return row.title;
    case SectionRole:
        return row.section;
    case CellsRole:
        return cellsFor(row);
    default:
        return {};
    }
}

QVariantMap EmojiGridModel::cellFor(const Row &row, const QString &cell) const
{
    if (row.kind == Kind::Kaomoji) {
        return {
            {u"text"_s, cell}, {u"base"_s, cell}, {u"name"_s, QString()}, {u"tones"_s, false}, {u"favorite"_s, m_store->isFavorite(cell)}};
    }
    return {
        {u"text"_s, row.stored ? cell : m_store->displayEmoji(cell)},
        {u"base"_s, m_store->baseEmoji(cell)},
        {u"name"_s, m_store->nameOf(cell)},
        {u"tones"_s, m_store->hasSkinTones(cell)},
        {u"favorite"_s, m_store->isFavorite(cell)},
    };
}

QVariantList EmojiGridModel::cellsFor(const Row &row) const
{
    QVariantList cells;
    cells.reserve(row.cells.size());
    for (const QString &cell : row.cells) {
        cells.append(cellFor(row, cell));
    }
    return cells;
}

QHash<int, QByteArray> EmojiGridModel::roleNames() const
{
    return {
        {KindRole, "kind"},
        {TitleRole, "title"},
        {SectionRole, "section"},
        {CellsRole, "cells"},
    };
}

int EmojiGridModel::columns() const
{
    return m_columns;
}

void EmojiGridModel::setColumns(int columns)
{
    columns = std::max(1, columns);
    if (columns == m_columns) {
        return;
    }
    m_columns = columns;
    Q_EMIT columnsChanged();
    rebuild();
}

int EmojiGridModel::kaomojiColumns() const
{
    return m_kaomojiColumns;
}

void EmojiGridModel::setKaomojiColumns(int columns)
{
    columns = std::max(1, columns);
    if (columns == m_kaomojiColumns) {
        return;
    }
    m_kaomojiColumns = columns;
    Q_EMIT kaomojiColumnsChanged();
    rebuild();
}

QString EmojiGridModel::searchText() const
{
    return m_searchText;
}

void EmojiGridModel::setSearchText(const QString &text)
{
    if (text == m_searchText) {
        return;
    }
    m_searchText = text;
    Q_EMIT searchTextChanged();
    rebuild();
}

QVariantList EmojiGridModel::sections() const
{
    QVariantList list;
    QString last;
    for (int i = 0; i < m_rows.size(); ++i) {
        if (m_rows.at(i).section != last) {
            last = m_rows.at(i).section;
            list.append(QVariantMap {{u"id"_s, last}, {u"row"_s, i}});
        }
    }
    return list;
}

bool EmojiGridModel::isStale() const
{
    return m_stale;
}

void EmojiGridModel::refresh()
{
    rebuild();
}

int EmojiGridModel::rowForSection(const QString &id) const
{
    for (int i = 0; i < m_rows.size(); ++i) {
        if (m_rows.at(i).section == id) {
            return i;
        }
    }
    return -1;
}

QString EmojiGridModel::sectionAt(int row) const
{
    if (m_rows.isEmpty()) {
        return {};
    }
    return m_rows.at(std::clamp(row, 0, int(m_rows.size()) - 1)).section;
}

void EmojiGridModel::markStale()
{
    if (!m_stale) {
        m_stale = true;
        Q_EMIT staleChanged();
    }
}

void EmojiGridModel::refreshCells()
{
    if (!m_rows.isEmpty()) {
        Q_EMIT dataChanged(index(0), index(int(m_rows.size()) - 1), {CellsRole});
    }
}

void EmojiGridModel::appendHeader(const QString &section, const QString &title)
{
    m_rows.append({Kind::Header, section, title, {}});
}

void EmojiGridModel::appendCells(const QString &section, const QStringList &cells, Kind kind, bool stored)
{
    const int perRow = kind == Kind::Kaomoji ? m_kaomojiColumns : m_columns;
    for (qsizetype i = 0; i < cells.size(); i += perRow) {
        m_rows.append({kind, section, {}, cells.mid(i, perRow), stored});
    }
}

void EmojiGridModel::appendSearchResults(const QString &query)
{
    const QStringList emoji = m_store->searchEmoji(query, maxEmojiResults);
    const QStringList kaomoji = m_store->searchKaomoji(query, maxKaomojiResults);
    const QString section = u"results"_s;
    appendCells(section, emoji, Kind::Emoji);
    if (!kaomoji.isEmpty()) {
        appendHeader(section, u"Kaomoji"_s);
        appendCells(section, kaomoji, Kind::Kaomoji);
    }
    if (emoji.isEmpty() && kaomoji.isEmpty()) {
        m_rows.append({Kind::Empty, section, u"No emoji found for “%1”"_s.arg(query), {}});
    }
}

void EmojiGridModel::appendRecents()
{
    const QString section = u"recents"_s;
    const QStringList recents = m_store->recentList();
    appendHeader(section, u"Recently used"_s);
    if (recents.isEmpty()) {
        m_rows.append({Kind::Empty, section, u"Emoji you use will show up here"_s, {}});
    }
    appendCells(section, recents, Kind::Emoji, true);

    QStringList favoriteEmoji;
    QStringList favoriteKaomoji;
    for (const QString &favorite : m_store->favoriteList()) {
        (m_store->isEmoji(favorite) ? favoriteEmoji : favoriteKaomoji).append(favorite);
    }
    if (!favoriteEmoji.isEmpty() || !favoriteKaomoji.isEmpty()) {
        appendHeader(section, u"Favourites"_s);
        appendCells(section, favoriteEmoji, Kind::Emoji);
        appendCells(section, favoriteKaomoji, Kind::Kaomoji);
    }
}

void EmojiGridModel::appendGroups()
{
    const EmojiData &data = m_store->data();
    QList<QStringList> byGroup(data.groups().size());
    for (const EmojiEntry &entry : data.entries()) {
        if (entry.supported) {
            byGroup[entry.group].append(entry.emoji);
        }
    }
    for (qsizetype group = 0; group < byGroup.size(); ++group) {
        const EmojiCategory &category = data.groups().at(group);
        appendHeader(category.id, category.name);
        appendCells(category.id, byGroup.at(group), Kind::Emoji);
    }
}

void EmojiGridModel::appendKaomoji()
{
    const EmojiData &data = m_store->data();
    QList<QStringList> byCategory(data.kaomojiCategories().size());
    for (const KaomojiEntry &entry : data.kaomoji()) {
        byCategory[entry.category].append(entry.text);
    }
    const QString section = u"kaomoji"_s;
    for (qsizetype category = 0; category < byCategory.size(); ++category) {
        appendHeader(section, data.kaomojiCategories().at(category).name);
        appendCells(section, byCategory.at(category), Kind::Kaomoji);
    }
}

void EmojiGridModel::rebuild()
{
    beginResetModel();
    m_rows.clear();
    const QString query = m_searchText.trimmed();
    if (m_store->isReady() && !query.isEmpty()) {
        appendSearchResults(query);
    } else if (m_store->isReady()) {
        appendRecents();
        appendGroups();
        appendKaomoji();
    }
    endResetModel();
    Q_EMIT sectionsChanged();
    if (m_stale) {
        m_stale = false;
        Q_EMIT staleChanged();
    }
}
