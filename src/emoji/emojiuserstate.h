#pragma once

#include <QHash>
#include <QList>
#include <QString>
#include <QStringList>

#include <optional>

class EmojiUserState
{
public:
    explicit EmojiUserState(QString directory);

    void load(int recentsLimit);

    QStringList recents() const;
    const QHash<QString, int> &recentUses() const { return m_uses; }
    void recordUse(const QString &text, int limit);
    bool trimRecents(int limit);
    void clearRecents();

    const QStringList &favorites() const { return m_favorites; }
    void toggleFavorite(const QString &key);

    std::optional<int> tone(const QString &base) const;
    bool setTone(const QString &base, int tone);

private:
    QString path(const QString &name) const;
    void loadRecents(int limit);
    void loadFavorites();
    void loadTones();
    void saveRecents() const;
    void saveFavorites() const;
    void saveTones() const;

    QString m_directory;
    QStringList m_recents;
    QHash<QString, int> m_uses;
    QStringList m_favorites;
    QHash<QString, int> m_tones;
};
