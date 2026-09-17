#include "emojidata.h"
#include "jsonfile.h"

#include <QJsonValue>
#include <QRawFont>
#include <QRegularExpression>

using namespace Qt::Literals::StringLiterals;

namespace
{
QString withoutSelectors(const QString &text)
{
    QString result = text;
    result.remove(QChar(0xFE0F));
    return result;
}

bool isJoiner(uint ucs4)
{
    return ucs4 == 0x200D || ucs4 == 0xFE0F || ucs4 == 0x20E3;
}
}

QStringList emojiWords(const QString &text)
{
    static const QRegularExpression separators(u"[\\s:,.()!’'\"-]+"_s);
    return text.toLower().split(separators, Qt::SkipEmptyParts);
}

bool EmojiData::fail(const QString &message)
{
    m_error = message;
    return false;
}

bool EmojiData::load(const QString &emojiPath, const QString &kaomojiPath, const QRawFont &font)
{
    const QJsonDocument emoji = JsonFile::read(emojiPath, &m_error);
    if (!m_error.isEmpty()) {
        return false;
    }
    parseEmoji(emoji.object());
    if (m_entries.isEmpty()) {
        return fail(u"%1 contains no emoji"_s.arg(emojiPath));
    }
    markSupported(font);

    const QJsonDocument kaomoji = JsonFile::read(kaomojiPath, &m_error);
    if (!m_error.isEmpty()) {
        return false;
    }
    parseKaomoji(kaomoji.object());
    return true;
}

void EmojiData::parseEmoji(const QJsonObject &root)
{
    m_version = root.value("emojiVersion"_L1).toString();
    for (const auto &value : root.value("groups"_L1).toArray()) {
        const QJsonObject group = value.toObject();
        m_groups.append({group.value("id"_L1).toString(), group.value("name"_L1).toString(), group.value("icon"_L1).toString()});
    }
    for (const auto &subgroup : root.value("subgroups"_L1).toArray()) {
        m_subgroupWords.append(emojiWords(subgroup.toString()));
    }
    const QJsonArray list = root.value("emoji"_L1).toArray();
    m_entries.reserve(list.size());
    for (const auto &row : list) {
        addEntry(row.toArray());
    }
    applyPopularity(root.value("popular"_L1).toArray());
}

void EmojiData::addEntry(const QJsonArray &row)
{
    EmojiEntry entry;
    entry.emoji = row.at(0).toString();
    entry.group = row.at(1).toInt();
    entry.subgroup = row.at(2).toInt();
    entry.name = row.at(3).toString();
    entry.lowerName = entry.name.toLower();
    entry.nameWords = emojiWords(entry.name);
    entry.keywords = row.at(4).toString().split(u'|', Qt::SkipEmptyParts);
    for (const auto &tone : row.at(6).toArray()) {
        entry.tones.append(tone.toString());
    }

    const auto index = int(m_entries.size());
    m_lookup.insert(entry.emoji, index);
    m_lookup.insert(withoutSelectors(entry.emoji), index);
    for (qsizetype tone = 0; tone < entry.tones.size(); ++tone) {
        for (const QString &form : {entry.tones.at(tone), withoutSelectors(entry.tones.at(tone))}) {
            m_lookup.insert(form, index);
            m_toneOf.insert(form, int(tone) + 1);
        }
    }
    m_entries.append(std::move(entry));
}

void EmojiData::applyPopularity(const QJsonArray &popular)
{
    for (qsizetype rank = 0; rank < popular.size(); ++rank) {
        const int index = indexOf(popular.at(rank).toString());
        if (index >= 0 && m_entries.at(index).popularity < 0) {
            m_entries[index].popularity = int(rank);
        }
    }
}

void EmojiData::markSupported(const QRawFont &font)
{
    m_supportedCount = 0;
    for (EmojiEntry &entry : m_entries) {
        const QList<uint> codepoints = entry.emoji.toUcs4();
        entry.supported = std::ranges::all_of(codepoints, [&font](uint ucs4) { return isJoiner(ucs4) || font.supportsCharacter(ucs4); });
        if (entry.supported) {
            m_supportedCount += 1 + int(entry.tones.size());
        }
    }
}

void EmojiData::parseKaomoji(const QJsonObject &root)
{
    for (const auto &value : root.value("categories"_L1).toArray()) {
        const QJsonObject category = value.toObject();
        const auto categoryIndex = int(m_kaomojiCategories.size());
        m_kaomojiCategories.append({category.value("id"_L1).toString(), category.value("name"_L1).toString(), {}});
        for (const auto &item : category.value("items"_L1).toArray()) {
            const QJsonArray pair = item.toArray();
            m_kaomoji.append({pair.at(0).toString(), emojiWords(pair.at(1).toString()), categoryIndex});
        }
    }
}
