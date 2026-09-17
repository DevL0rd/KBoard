#include "emojistore.h"
#include "emojilistmodel.h"
#include "emojimatcher.h"

#include "kboardpaths.h"
#include "kboardsettings.h"

#include <QFont>
#include <QRawFont>

using namespace Qt::Literals::StringLiterals;

namespace
{
const QString emojiFontFamily = u"Noto Color Emoji"_s;
const QStringList toneNames
    = {u"light skin tone"_s, u"medium-light skin tone"_s, u"medium skin tone"_s, u"medium-dark skin tone"_s, u"dark skin tone"_s};
EmojiStore *s_instance = nullptr;

QVariantMap categoryMap(const QString &id, const QString &name, const QString &icon)
{
    return {{u"id"_s, id}, {u"name"_s, name}, {u"icon"_s, icon}};
}
}

EmojiStore::EmojiStore(QObject *parent)
    : QObject(parent)
    , m_state(KBoardPaths::userDataFile(u"emoji"_s))
    , m_recentsModel(new EmojiListModel(this, EmojiListModel::Display::AsStored, this))
    , m_favoritesModel(new EmojiListModel(this, EmojiListModel::Display::PreferredTone, this))
{
    load();
    connect(KBoardSettings::self(), &KBoardSettings::skinToneChanged, this, &EmojiStore::tonesChanged);
    connect(KBoardSettings::self(), &KBoardSettings::emojiRecentsLimitChanged, this, &EmojiStore::onRecentsLimitChanged);
}

EmojiStore::~EmojiStore()
{
    if (s_instance == this) {
        s_instance = nullptr;
    }
}

EmojiStore *EmojiStore::instance()
{
    if (!s_instance) {
        s_instance = new EmojiStore;
    }
    return s_instance;
}

EmojiStore *EmojiStore::create(QQmlEngine *, QJSEngine *)
{
    QJSEngine::setObjectOwnership(instance(), QJSEngine::CppOwnership);
    return instance();
}

void EmojiStore::load()
{
    const QRawFont font = QRawFont::fromFont(QFont(emojiFontFamily));
    if (!font.isValid() || font.familyName() != emojiFontFamily) {
        m_error = u"The %1 font is not installed (package noto-fonts-emoji)"_s.arg(emojiFontFamily);
    } else if (!m_data.load(KBoardPaths::dataFile(u"emoji/emoji.json"_s), KBoardPaths::dataFile(u"emoji/kaomoji.json"_s), font)) {
        m_error = m_data.errorString();
    }
    if (!m_error.isEmpty()) {
        qWarning("EmojiStore: %s", qPrintable(m_error));
        return;
    }
    m_state.load(KBoardSettings::emojiRecentsLimit());
    m_recentsModel->setEmoji(m_state.recents());
    m_favoritesModel->setEmoji(m_state.favorites());
    m_ready = true;
}

void EmojiStore::onRecentsLimitChanged()
{
    if (m_state.trimRecents(KBoardSettings::emojiRecentsLimit())) {
        m_recentsModel->setEmoji(m_state.recents());
        Q_EMIT recentsChanged();
    }
}

QVariantList EmojiStore::groups() const
{
    QVariantList list {categoryMap(u"recents"_s, u"Recently used"_s, u"🕘"_s)};
    for (const EmojiCategory &group : m_data.groups()) {
        list.append(categoryMap(group.id, group.name, group.icon));
    }
    list.append(categoryMap(u"kaomoji"_s, u"Kaomoji"_s, u"ツ"_s));
    return list;
}

QVariantList EmojiStore::kaomojiCategories() const
{
    QVariantList list;
    for (const EmojiCategory &category : m_data.kaomojiCategories()) {
        list.append(categoryMap(category.id, category.name, category.icon));
    }
    return list;
}

QAbstractItemModel *EmojiStore::recentsModel() const
{
    return m_recentsModel;
}

QAbstractItemModel *EmojiStore::favoritesModel() const
{
    return m_favoritesModel;
}

const EmojiEntry *EmojiStore::entryFor(const QString &emoji) const
{
    const int index = m_data.indexOf(emoji);
    return index < 0 ? nullptr : &m_data.entries().at(index);
}

QHash<int, int> EmojiStore::usageByEntry() const
{
    QHash<int, int> usage;
    const QHash<QString, int> &uses = m_state.recentUses();
    for (auto it = uses.constBegin(); it != uses.constEnd(); ++it) {
        const int index = m_data.indexOf(it.key());
        if (index >= 0) {
            usage[index] += it.value();
        }
    }
    return usage;
}

QStringList EmojiStore::emojiAt(const QList<int> &indexes) const
{
    QStringList result;
    result.reserve(indexes.size());
    for (int index : indexes) {
        result.append(m_data.entries().at(index).emoji);
    }
    return result;
}

QAbstractItemModel *EmojiStore::modelForGroup(const QString &id)
{
    if (id == "recents"_L1) {
        return m_recentsModel;
    }
    if (id == "favorites"_L1) {
        return m_favoritesModel;
    }
    if (EmojiListModel *model = m_groupModels.value(id)) {
        return model;
    }
    const QList<EmojiCategory> &groups = m_data.groups();
    const auto group = std::ranges::find(groups, id, &EmojiCategory::id);
    if (group == groups.end()) {
        qWarning("EmojiStore: unknown emoji group %s", qPrintable(id));
        return nullptr;
    }
    const auto groupIndex = int(std::distance(groups.begin(), group));
    QStringList emoji;
    for (const EmojiEntry &entry : m_data.entries()) {
        if (entry.group == groupIndex && entry.supported) {
            emoji.append(entry.emoji);
        }
    }
    auto *model = new EmojiListModel(this, EmojiListModel::Display::PreferredTone, this);
    model->setEmoji(emoji);
    m_groupModels.insert(id, model);
    return model;
}

QAbstractItemModel *EmojiStore::search(const QString &query)
{
    auto *model = new EmojiListModel(this, EmojiListModel::Display::PreferredTone);
    model->setEmoji(searchEmoji(query, 200));
    QQmlEngine::setObjectOwnership(model, QQmlEngine::JavaScriptOwnership);
    return model;
}

QStringList EmojiStore::searchEmoji(const QString &query, int limit) const
{
    return emojiAt(EmojiMatcher(m_data, usageByEntry()).search(query, limit));
}

QStringList EmojiStore::searchKaomoji(const QString &query, int limit) const
{
    QStringList result;
    for (int index : EmojiMatcher(m_data, {}).searchKaomoji(query, limit)) {
        result.append(m_data.kaomoji().at(index).text);
    }
    return result;
}

QStringList EmojiStore::suggestionsFor(const QString &word, int limit) const
{
    QStringList result = emojiAt(EmojiMatcher(m_data, usageByEntry()).suggest(word, limit));
    for (QString &emoji : result) {
        emoji = displayEmoji(emoji);
    }
    return result;
}

void EmojiStore::recordUse(const QString &emoji)
{
    if (emoji.isEmpty()) {
        return;
    }
    m_state.recordUse(emoji, KBoardSettings::emojiRecentsLimit());
    m_recentsModel->setEmoji(m_state.recents());
    Q_EMIT recentsChanged();
    if (hasSkinTones(emoji)) {
        setPreferredTone(emoji, skinToneOf(emoji));
    }
}

void EmojiStore::clearRecents()
{
    m_state.clearRecents();
    m_recentsModel->setEmoji({});
    Q_EMIT recentsChanged();
}

void EmojiStore::toggleFavorite(const QString &emoji)
{
    if (emoji.isEmpty()) {
        return;
    }
    m_state.toggleFavorite(baseEmoji(emoji));
    m_favoritesModel->setEmoji(m_state.favorites());
    Q_EMIT favoritesChanged();
}

bool EmojiStore::isFavorite(const QString &emoji) const
{
    return m_state.favorites().contains(baseEmoji(emoji));
}

int EmojiStore::usageCount(const QString &emoji) const
{
    const int index = m_data.indexOf(emoji);
    return index < 0 ? m_state.recentUses().value(emoji) : usageByEntry().value(index);
}

QString EmojiStore::withSkinTone(const QString &emoji, int tone) const
{
    const EmojiEntry *entry = entryFor(emoji);
    if (!entry || entry->tones.isEmpty()) {
        return emoji;
    }
    return tone <= 0 || tone > entry->tones.size() ? entry->emoji : entry->tones.at(tone - 1);
}

QStringList EmojiStore::skinToneVariants(const QString &emoji) const
{
    const EmojiEntry *entry = entryFor(emoji);
    if (!entry || entry->tones.isEmpty()) {
        return {};
    }
    return QStringList {entry->emoji} + entry->tones;
}

bool EmojiStore::hasSkinTones(const QString &emoji) const
{
    const EmojiEntry *entry = entryFor(emoji);
    return entry && !entry->tones.isEmpty();
}

QString EmojiStore::baseEmoji(const QString &emoji) const
{
    const EmojiEntry *entry = entryFor(emoji);
    return entry ? entry->emoji : emoji;
}

int EmojiStore::skinToneOf(const QString &emoji) const
{
    return m_data.toneOf(emoji);
}

int EmojiStore::preferredTone(const QString &emoji) const
{
    return m_state.tone(baseEmoji(emoji)).value_or(KBoardSettings::skinTone());
}

void EmojiStore::setPreferredTone(const QString &emoji, int tone)
{
    if (hasSkinTones(emoji) && tone != preferredTone(emoji) && m_state.setTone(baseEmoji(emoji), tone)) {
        Q_EMIT tonesChanged();
    }
}

QString EmojiStore::displayEmoji(const QString &emoji) const
{
    return hasSkinTones(emoji) ? withSkinTone(emoji, preferredTone(emoji)) : emoji;
}

QString EmojiStore::nameOf(const QString &emoji) const
{
    const EmojiEntry *entry = entryFor(emoji);
    if (!entry) {
        return {};
    }
    const int tone = skinToneOf(emoji);
    return tone > 0 ? entry->name + u": "_s + toneNames.at(tone - 1) : entry->name;
}

bool EmojiStore::isEmoji(const QString &emoji) const
{
    return entryFor(emoji) != nullptr;
}
