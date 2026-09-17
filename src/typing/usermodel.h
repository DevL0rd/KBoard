#pragma once

#include <QHash>
#include <QSet>
#include <QString>
#include <QStringList>

class UserModel
{
public:
    struct Word
    {
        QString form;
        int count = 0;
        qint64 lastUsed = 0;
        bool pinned = false;
    };

    static constexpr int KnownThreshold = 2;

    void clear();
    QByteArray serialize() const;
    bool deserialize(const QByteArray &data);

    void learnWord(const QString &form, int weight, bool pinned, qint64 now);
    void learnSequence(const QStringList &contextKeys, const QString &key);
    void forget(const QString &key);

    bool isBlocked(const QString &key) const;
    bool isKnown(const QString &key) const;
    const Word *word(const QString &key) const;
    const QHash<QString, Word> &words() const { return m_words; }
    const QHash<QString, int> *followers(const QString &context) const;

    void rejectCorrection(const QString &originalKey, const QString &correctionKey);
    bool isRejected(const QString &originalKey, const QString &correctionKey) const;

    QStringList knownWords() const;

private:
    QHash<QString, Word> m_words;
    QHash<QString, QHash<QString, int>> m_followers;
    QSet<QString> m_blocked;
    QSet<QString> m_rejected;
};
