#pragma once

#include <QHash>
#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QString>
#include <QStringList>

class QRawFont;

struct EmojiEntry
{
    QString emoji;
    int group = 0;
    int subgroup = 0;
    QString name;
    QString lowerName;
    QStringList nameWords;
    QStringList keywords;
    QStringList tones;
    int popularity = -1;
    bool supported = true;
};

struct KaomojiEntry
{
    QString text;
    QStringList keywords;
    int category = 0;
};

struct EmojiCategory
{
    QString id;
    QString name;
    QString icon;
};

QStringList emojiWords(const QString &text);

class EmojiData
{
public:
    static constexpr int toneCount = 5;

    bool load(const QString &emojiPath, const QString &kaomojiPath, const QRawFont &font);

    QString errorString() const { return m_error; }
    QString version() const { return m_version; }
    int supportedCount() const { return m_supportedCount; }
    const QList<EmojiEntry> &entries() const { return m_entries; }
    const QList<KaomojiEntry> &kaomoji() const { return m_kaomoji; }
    const QList<EmojiCategory> &groups() const { return m_groups; }
    const QList<EmojiCategory> &kaomojiCategories() const { return m_kaomojiCategories; }
    QStringList subgroupWords(int subgroup) const { return m_subgroupWords.value(subgroup); }

    int indexOf(const QString &emoji) const { return m_lookup.value(emoji, -1); }
    int toneOf(const QString &emoji) const { return m_toneOf.value(emoji, 0); }

private:
    bool fail(const QString &message);
    void parseEmoji(const QJsonObject &root);
    void addEntry(const QJsonArray &row);
    void applyPopularity(const QJsonArray &popular);
    void markSupported(const QRawFont &font);
    void parseKaomoji(const QJsonObject &root);

    QString m_error;
    QString m_version;
    QList<EmojiEntry> m_entries;
    QList<KaomojiEntry> m_kaomoji;
    QList<EmojiCategory> m_groups;
    QList<EmojiCategory> m_kaomojiCategories;
    QList<QStringList> m_subgroupWords;
    QHash<QString, int> m_lookup;
    QHash<QString, int> m_toneOf;
    int m_supportedCount = 0;
};
