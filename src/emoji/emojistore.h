#pragma once

#include "emojidata.h"
#include "emojiuserstate.h"

#include <QAbstractItemModel>
#include <QHash>
#include <QObject>
#include <QQmlEngine>
#include <QVariantList>

class EmojiListModel;
class EmojiMatcher;

class EmojiStore : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(bool ready READ isReady CONSTANT)
    Q_PROPERTY(QString errorString READ errorString CONSTANT)
    Q_PROPERTY(QString emojiVersion READ emojiVersion CONSTANT)
    Q_PROPERTY(int count READ count CONSTANT)
    Q_PROPERTY(QVariantList groups READ groups CONSTANT)
    Q_PROPERTY(QVariantList kaomojiCategories READ kaomojiCategories CONSTANT)
    Q_PROPERTY(QAbstractItemModel *recents READ recentsModel CONSTANT)
    Q_PROPERTY(QAbstractItemModel *favorites READ favoritesModel CONSTANT)
    Q_PROPERTY(QStringList recentList READ recentList NOTIFY recentsChanged)
    Q_PROPERTY(QStringList favoriteList READ favoriteList NOTIFY favoritesChanged)

public:
    explicit EmojiStore(QObject *parent = nullptr);
    ~EmojiStore() override;

    static EmojiStore *instance();
    static EmojiStore *create(QQmlEngine *, QJSEngine *);

    bool isReady() const { return m_ready; }
    QString errorString() const { return m_error; }
    QString emojiVersion() const { return m_data.version(); }
    int count() const { return m_data.supportedCount(); }
    QVariantList groups() const;
    QVariantList kaomojiCategories() const;
    QAbstractItemModel *recentsModel() const;
    QAbstractItemModel *favoritesModel() const;
    QStringList recentList() const { return m_state.recents(); }
    QStringList favoriteList() const { return m_state.favorites(); }
    const EmojiData &data() const { return m_data; }

    Q_INVOKABLE QAbstractItemModel *modelForGroup(const QString &id);
    Q_INVOKABLE QAbstractItemModel *search(const QString &query);
    Q_INVOKABLE QStringList searchEmoji(const QString &query, int limit = 60) const;
    Q_INVOKABLE QStringList searchKaomoji(const QString &query, int limit = 30) const;
    Q_INVOKABLE QStringList suggestionsFor(const QString &word, int limit = 3) const;

    Q_INVOKABLE void recordUse(const QString &emoji);
    Q_INVOKABLE void clearRecents();
    Q_INVOKABLE void toggleFavorite(const QString &emoji);
    Q_INVOKABLE bool isFavorite(const QString &emoji) const;
    Q_INVOKABLE int usageCount(const QString &emoji) const;

    Q_INVOKABLE QString withSkinTone(const QString &emoji, int tone) const;
    Q_INVOKABLE QStringList skinToneVariants(const QString &emoji) const;
    Q_INVOKABLE bool hasSkinTones(const QString &emoji) const;
    Q_INVOKABLE QString baseEmoji(const QString &emoji) const;
    Q_INVOKABLE int skinToneOf(const QString &emoji) const;
    Q_INVOKABLE int preferredTone(const QString &emoji) const;
    Q_INVOKABLE void setPreferredTone(const QString &emoji, int tone);
    Q_INVOKABLE QString displayEmoji(const QString &emoji) const;
    Q_INVOKABLE QString nameOf(const QString &emoji) const;
    Q_INVOKABLE bool isEmoji(const QString &emoji) const;

Q_SIGNALS:
    void recentsChanged();
    void favoritesChanged();
    void tonesChanged();

private:
    void load();
    void onRecentsLimitChanged();
    const EmojiEntry *entryFor(const QString &emoji) const;
    QHash<int, int> usageByEntry() const;
    QStringList emojiAt(const QList<int> &indexes) const;

    EmojiData m_data;
    EmojiUserState m_state;
    bool m_ready = false;
    QString m_error;
    EmojiListModel *m_recentsModel;
    EmojiListModel *m_favoritesModel;
    QHash<QString, EmojiListModel *> m_groupModels;
};
