#include "settingscatalog.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>

#include <algorithm>

SettingsCatalog::SettingsCatalog(QObject *parent)
    : SettingsCatalog(QStringLiteral(":/kboard/settings/index.json"), parent)
{ }

SettingsCatalog::SettingsCatalog(const QString &path, QObject *parent)
    : QObject(parent)
{
    load(path);
}

void SettingsCatalog::load(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        m_errorString = QStringLiteral("The settings index %1 is missing.").arg(path);
        return;
    }
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        m_errorString = QStringLiteral("The settings index is broken: %1").arg(parseError.errorString());
        return;
    }
    const QJsonObject root = document.object();
    m_pages = root.value(QStringLiteral("pages")).toArray().toVariantList();
    QVariantMap titles;
    QVariantMap icons;
    for (const QVariant &page : std::as_const(m_pages)) {
        const QVariantMap map = page.toMap();
        titles.insert(map.value(QStringLiteral("id")).toString(), map.value(QStringLiteral("title")));
        icons.insert(map.value(QStringLiteral("id")).toString(), map.value(QStringLiteral("icon")));
    }
    const QJsonArray entries = root.value(QStringLiteral("entries")).toArray();
    for (const auto &value : entries) {
        QVariantMap map = value.toObject().toVariantMap();
        const QString pageId = map.value(QStringLiteral("page")).toString();
        map.insert(QStringLiteral("pageTitle"), titles.value(pageId));
        map.insert(QStringLiteral("icon"), icons.value(pageId));
        m_entries.append(map);
    }
}

QVariantList SettingsCatalog::pages() const
{
    return m_pages;
}

QVariantList SettingsCatalog::entries() const
{
    return m_entries;
}

QString SettingsCatalog::errorString() const
{
    return m_errorString;
}

QVariantMap SettingsCatalog::page(const QString &id) const
{
    for (const QVariant &page : m_pages) {
        const QVariantMap map = page.toMap();
        if (map.value(QStringLiteral("id")).toString() == id) {
            return map;
        }
    }
    return {};
}

bool SettingsCatalog::hasPage(const QString &id) const
{
    return !page(id).isEmpty();
}

QVariantList SettingsCatalog::entriesForPage(const QString &id) const
{
    QVariantList result;
    for (const QVariant &entry : m_entries) {
        if (entry.toMap().value(QStringLiteral("page")).toString() == id) {
            result.append(entry);
        }
    }
    return result;
}

QVariantList SettingsCatalog::search(const QString &query, int limit) const
{
    static const QRegularExpression spaces(QStringLiteral("\\s+"));
    const QStringList words = query.toLower().split(spaces, Qt::SkipEmptyParts);
    if (words.isEmpty()) {
        return {};
    }
    struct Hit
    {
        int score;
        int order;
        QVariantMap entry;
    };
    QList<Hit> hits;
    int order = 0;
    for (const QVariant &value : m_entries) {
        const QVariantMap entry = value.toMap();
        const QString label = entry.value(QStringLiteral("label")).toString().toLower();
        const QString haystack = label + QLatin1Char(' ') + entry.value(QStringLiteral("section")).toString().toLower() + QLatin1Char(' ')
            + entry.value(QStringLiteral("pageTitle")).toString().toLower() + QLatin1Char(' ')
            + entry.value(QStringLiteral("keywords")).toString().toLower() + QLatin1Char(' ')
            + entry.value(QStringLiteral("settings")).toStringList().join(QLatin1Char(' ')).toLower();
        const bool all = std::all_of(words.cbegin(), words.cend(), [&haystack](const QString &word) { return haystack.contains(word); });
        if (!all) {
            continue;
        }
        int score = 0;
        for (const QString &word : words) {
            if (label.startsWith(word)) {
                score += 4;
            } else if (label.contains(word)) {
                score += 2;
            }
        }
        hits.append({score, order++, entry});
    }
    std::stable_sort(hits.begin(), hits.end(), [](const Hit &a, const Hit &b) { return a.score > b.score; });
    QVariantList result;
    for (const Hit &hit : std::as_const(hits)) {
        if (result.size() >= limit) {
            break;
        }
        result.append(hit.entry);
    }
    return result;
}
