#include "emojiuserstate.h"
#include "jsonfile.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonObject>

#include <algorithm>

using namespace Qt::Literals::StringLiterals;

namespace
{
constexpr int maxTone = 5;

QJsonDocument readState(const QString &path)
{
    if (!QFile::exists(path)) {
        return {};
    }
    QString error;
    QJsonDocument document = JsonFile::read(path, &error);
    if (!error.isEmpty()) {
        qWarning("EmojiUserState: %s", qPrintable(error));
    }
    return document;
}
}

EmojiUserState::EmojiUserState(QString directory)
    : m_directory(std::move(directory))
{ }

QString EmojiUserState::path(const QString &name) const
{
    return m_directory + u'/' + name;
}

void EmojiUserState::load(int recentsLimit)
{
    loadRecents(recentsLimit);
    loadFavorites();
    loadTones();
}

void EmojiUserState::loadRecents(int limit)
{
    const QJsonArray array = readState(path(u"recents.json"_s)).array();
    for (const auto &value : array) {
        const QJsonObject object = value.toObject();
        const QString emoji = object.value("emoji"_L1).toString();
        if (!emoji.isEmpty() && !m_uses.contains(emoji)) {
            m_recents.append(emoji);
            m_uses.insert(emoji, std::max(1, object.value("uses"_L1).toInt()));
        }
    }
    trimRecents(limit);
}

void EmojiUserState::loadFavorites()
{
    const QJsonArray array = readState(path(u"favorites.json"_s)).array();
    for (const auto &value : array) {
        m_favorites.append(value.toString());
    }
}

void EmojiUserState::loadTones()
{
    const QJsonObject object = readState(path(u"skintones.json"_s)).object();
    for (auto it = object.constBegin(); it != object.constEnd(); ++it) {
        m_tones.insert(it.key(), std::clamp(it.value().toInt(), 0, maxTone));
    }
}

void EmojiUserState::saveRecents() const
{
    QJsonArray array;
    for (const QString &emoji : m_recents) {
        array.append(QJsonObject {{"emoji"_L1, emoji}, {"uses"_L1, m_uses.value(emoji)}});
    }
    JsonFile::write(path(u"recents.json"_s), QJsonDocument(array));
}

void EmojiUserState::saveFavorites() const
{
    JsonFile::write(path(u"favorites.json"_s), QJsonDocument(QJsonArray::fromStringList(m_favorites)));
}

void EmojiUserState::saveTones() const
{
    QJsonObject object;
    for (auto it = m_tones.constBegin(); it != m_tones.constEnd(); ++it) {
        object.insert(it.key(), it.value());
    }
    JsonFile::write(path(u"skintones.json"_s), QJsonDocument(object));
}

QStringList EmojiUserState::recents() const
{
    return m_recents;
}

void EmojiUserState::recordUse(const QString &text, int limit)
{
    m_recents.removeOne(text);
    m_recents.prepend(text);
    m_uses[text] += 1;
    trimRecents(limit);
    saveRecents();
}

bool EmojiUserState::trimRecents(int limit)
{
    if (m_recents.size() <= limit) {
        return false;
    }
    for (qsizetype i = limit; i < m_recents.size(); ++i) {
        m_uses.remove(m_recents.at(i));
    }
    m_recents.resize(limit);
    saveRecents();
    return true;
}

void EmojiUserState::clearRecents()
{
    m_recents.clear();
    m_uses.clear();
    saveRecents();
}

void EmojiUserState::toggleFavorite(const QString &key)
{
    if (!m_favorites.removeOne(key)) {
        m_favorites.prepend(key);
    }
    saveFavorites();
}

std::optional<int> EmojiUserState::tone(const QString &base) const
{
    const auto it = m_tones.constFind(base);
    return it == m_tones.constEnd() ? std::nullopt : std::optional<int>(it.value());
}

bool EmojiUserState::setTone(const QString &base, int tone)
{
    tone = std::clamp(tone, 0, maxTone);
    if (m_tones.value(base, -1) == tone) {
        return false;
    }
    m_tones.insert(base, tone);
    saveTones();
    return true;
}
