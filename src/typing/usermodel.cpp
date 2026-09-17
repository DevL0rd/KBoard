#include "usermodel.h"

#include <QList>

#include <algorithm>

namespace
{
const QChar Separator = QChar(u'\t');

QString pairKey(const QString &a, const QString &b)
{
    return a + Separator + b;
}
}

void UserModel::clear()
{
    m_words.clear();
    m_followers.clear();
    m_blocked.clear();
    m_rejected.clear();
}

QByteArray UserModel::serialize() const
{
    QString out = QStringLiteral("#kboard-user 1\n");
    QStringList keys = m_words.keys();
    std::sort(keys.begin(), keys.end());
    for (const QString &key : std::as_const(keys)) {
        const Word &word = m_words[key];
        out += QStringLiteral("w\t%1\t%2\t%3\t%4\n").arg(word.form).arg(word.count).arg(word.lastUsed).arg(word.pinned ? 1 : 0);
    }
    QStringList contexts = m_followers.keys();
    std::sort(contexts.begin(), contexts.end());
    for (const QString &context : std::as_const(contexts)) {
        const QHash<QString, int> &next = m_followers[context];
        for (auto it = next.cbegin(); it != next.cend(); ++it) {
            out += QStringLiteral("n\t%1\t%2\t%3\n").arg(context, it.key()).arg(it.value());
        }
    }
    for (const QString &key : m_blocked) {
        out += QStringLiteral("b\t%1\n").arg(key);
    }
    for (const QString &pair : m_rejected) {
        out += QStringLiteral("r\t%1\n").arg(pair);
    }
    return out.toUtf8();
}

bool UserModel::deserialize(const QByteArray &data)
{
    clear();
    if (data.isEmpty()) {
        return true;
    }
    if (!data.startsWith("#kboard-user 1")) {
        return false;
    }
    const QStringList lines = QString::fromUtf8(data).split(u'\n', Qt::SkipEmptyParts);
    for (const QString &line : lines) {
        const QStringList fields = line.split(Separator);
        const QString &type = fields.first();
        if (type == u"w" && fields.size() == 5) {
            Word word;
            word.form = fields[1];
            word.count = fields[2].toInt();
            word.lastUsed = fields[3].toLongLong();
            word.pinned = fields[4] == u"1";
            m_words.insert(word.form.toLower(), word);
        } else if (type == u"n" && fields.size() == 4) {
            m_followers[fields[1]][fields[2]] = fields[3].toInt();
        } else if (type == u"b" && fields.size() == 2) {
            m_blocked.insert(fields[1]);
        } else if (type == u"r" && fields.size() == 3) {
            m_rejected.insert(pairKey(fields[1], fields[2]));
        }
    }
    return true;
}

void UserModel::learnWord(const QString &form, int weight, bool pinned, qint64 now)
{
    const QString key = form.toLower();
    m_blocked.remove(key);
    Word &word = m_words[key];
    if (word.form.isEmpty() || form != key || word.count == 0) {
        word.form = form;
    }
    word.count += weight;
    word.lastUsed = now;
    word.pinned = word.pinned || pinned;
}

void UserModel::learnSequence(const QStringList &contextKeys, const QString &key)
{
    if (contextKeys.isEmpty()) {
        return;
    }
    m_followers[contextKeys.last()][key] += 1;
    if (contextKeys.size() >= 2) {
        m_followers[contextKeys[contextKeys.size() - 2] + u' ' + contextKeys.last()][key] += 1;
    }
}

void UserModel::forget(const QString &key)
{
    m_words.remove(key);
    m_blocked.insert(key);
    for (auto it = m_followers.begin(); it != m_followers.end();) {
        it->remove(key);
        const QStringList parts = it.key().split(u' ');
        if (it->isEmpty() || parts.contains(key)) {
            it = m_followers.erase(it);
        } else {
            ++it;
        }
    }
}

bool UserModel::isBlocked(const QString &key) const
{
    return m_blocked.contains(key);
}

bool UserModel::isKnown(const QString &key) const
{
    auto it = m_words.constFind(key);
    return it != m_words.cend() && (it->pinned || it->count >= KnownThreshold);
}

const UserModel::Word *UserModel::word(const QString &key) const
{
    auto it = m_words.constFind(key);
    return it == m_words.cend() ? nullptr : &it.value();
}

const QHash<QString, int> *UserModel::followers(const QString &context) const
{
    auto it = m_followers.constFind(context);
    return it == m_followers.cend() ? nullptr : &it.value();
}

void UserModel::rejectCorrection(const QString &originalKey, const QString &correctionKey)
{
    m_rejected.insert(pairKey(originalKey, correctionKey));
}

bool UserModel::isRejected(const QString &originalKey, const QString &correctionKey) const
{
    return m_rejected.contains(pairKey(originalKey, correctionKey));
}

QStringList UserModel::knownWords() const
{
    QStringList result;
    for (auto it = m_words.cbegin(); it != m_words.cend(); ++it) {
        if (it->pinned || it->count >= KnownThreshold) {
            result.append(it->form);
        }
    }
    std::sort(result.begin(), result.end(), [](const QString &a, const QString &b) { return a.compare(b, Qt::CaseInsensitive) < 0; });
    return result;
}
