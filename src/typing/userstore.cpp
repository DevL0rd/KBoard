#include "userstore.h"

#include "kboardpaths.h"
#include "textanalysis.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <QThreadPool>
#include <QUrl>

namespace
{
constexpr int SaveDelayMs = 2000;
constexpr int MaxWordLength = 48;

QThreadPool *savePool()
{
    static QThreadPool *pool = [] {
        auto *created = new QThreadPool;
        created->setMaxThreadCount(1);
        return created;
    }();
    return pool;
}

void writeFile(const QString &path, const QByteArray &data)
{
    QDir().mkpath(QFileInfo(path).absolutePath());
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning("TypingEngine: cannot write %s: %s", qPrintable(path), qPrintable(file.errorString()));
        return;
    }
    file.write(data);
    if (!file.commit()) {
        qWarning("TypingEngine: cannot save %s: %s", qPrintable(path), qPrintable(file.errorString()));
    }
}

QString localPath(const QString &filePath)
{
    return filePath.startsWith(QStringLiteral("file:")) ? QUrl(filePath).toLocalFile() : filePath;
}
}

UserStore::UserStore(QObject *parent)
    : QObject(parent)
{
    m_saveTimer.setSingleShot(true);
    m_saveTimer.setInterval(SaveDelayMs);
    connect(&m_saveTimer, &QTimer::timeout, this, &UserStore::save);
}

UserStore::~UserStore()
{
    flush();
}

UserModel &UserStore::model()
{
    return m_model;
}

const UserModel &UserStore::model() const
{
    return m_model;
}

QString UserStore::pathFor(const QString &language)
{
    return KBoardPaths::userDataFile(QStringLiteral("typing/%1/user.tsv").arg(language));
}

QString UserStore::open(const QString &language, const QByteArray &data)
{
    close();
    m_language = language;
    if (m_model.deserialize(data)) {
        return QString();
    }
    m_model.clear();
    const QString path = pathFor(language);
    const QString moved = path + QStringLiteral(".unreadable");
    QFile::remove(moved);
    QFile::rename(path, moved);
    return QStringLiteral("Learned words file had an unknown format and was moved to %1").arg(moved);
}

void UserStore::close()
{
    flush();
    m_model.clear();
    m_language.clear();
}

void UserStore::markChanged()
{
    m_dirty = true;
    m_saveTimer.start();
}

void UserStore::flush()
{
    m_saveTimer.stop();
    save();
    savePool()->waitForDone();
}

void UserStore::save()
{
    if (!m_dirty || m_language.isEmpty()) {
        return;
    }
    m_dirty = false;
    savePool()->start([path = pathFor(m_language), data = m_model.serialize()] { writeFile(path, data); });
}

bool UserStore::exportWords(const QString &filePath) const
{
    QSaveFile file(localPath(filePath));
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    file.write((m_model.knownWords().join(u'\n') + u'\n').toUtf8());
    return file.commit();
}

int UserStore::importWords(const QString &filePath)
{
    QFile file(localPath(filePath));
    if (m_language.isEmpty() || !file.open(QIODevice::ReadOnly)) {
        return -1;
    }
    int imported = 0;
    const qint64 now = QDateTime::currentSecsSinceEpoch();
    const QStringList lines = QString::fromUtf8(file.readAll()).split(u'\n', Qt::SkipEmptyParts);
    for (const QString &line : lines) {
        const QString word = TextAnalysis::stripPunctuation(line.trimmed());
        if (word.isEmpty() || word.size() > MaxWordLength || word.contains(u' ')) {
            continue;
        }
        m_model.learnWord(word, UserModel::KnownThreshold, true, now);
        ++imported;
    }
    if (imported > 0) {
        markChanged();
    }
    return imported;
}
