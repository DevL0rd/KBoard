#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>
#include <QStringList>

class EmojiStore;

class EmojiListModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Obtained from EmojiStore")

    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

public:
    enum Role
    {
        EmojiRole = Qt::UserRole + 1,
        BaseEmojiRole,
        NameRole,
        HasSkinTonesRole,
        FavoriteRole,
        GroupRole,
    };
    Q_ENUM(Role)

    enum class Display
    {
        PreferredTone,
        AsStored,
    };

    EmojiListModel(EmojiStore *store, Display display, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setEmoji(const QStringList &emoji);
    Q_INVOKABLE QString get(int row) const;

Q_SIGNALS:
    void countChanged();

private:
    void refreshAll();

    EmojiStore *m_store;
    Display m_display;
    QStringList m_emoji;
};
