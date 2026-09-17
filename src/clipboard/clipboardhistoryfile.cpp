#include "clipboardhistoryfile.h"

#include <KLocalizedString>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSaveFile>

ClipboardHistoryFile::ClipboardHistoryFile(QString path)
    : m_path(std::move(path))
{
    ensurePrivateDirectory(QFileInfo(m_path).absolutePath());
}

void ClipboardHistoryFile::ensurePrivateDirectory(const QString &path)
{
    QDir().mkpath(path);
    QFile::setPermissions(path, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);
}

QString ClipboardHistoryFile::path() const
{
    return m_path;
}

QString ClipboardHistoryFile::errorString() const
{
    return m_errorString;
}

bool ClipboardHistoryFile::save(const QList<ClipboardEntry> &entries)
{
    QJsonArray items;
    for (const ClipboardEntry &entry : entries) {
        items.append(entry.toJson());
    }
    QSaveFile file(m_path);
    if (!file.open(QIODevice::WriteOnly)) {
        m_errorString = i18n("Cannot save clipboard history: %1", file.errorString());
        return false;
    }
    file.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner);
    file.write(
        QJsonDocument(QJsonObject {{QStringLiteral("version"), 1}, {QStringLiteral("items"), items}}).toJson(QJsonDocument::Compact));
    if (!file.commit()) {
        m_errorString = i18n("Cannot save clipboard history: %1", file.errorString());
        return false;
    }
    m_errorString.clear();
    return true;
}

std::optional<QList<ClipboardEntry>> ClipboardHistoryFile::load()
{
    QFile file(m_path);
    if (!file.exists()) {
        return QList<ClipboardEntry>();
    }
    if (!file.open(QIODevice::ReadOnly)) {
        m_errorString = i18n("Cannot read clipboard history: %1", file.errorString());
        return std::nullopt;
    }
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        m_errorString = i18n("Clipboard history file is damaged: %1", parseError.errorString());
        return std::nullopt;
    }
    QList<ClipboardEntry> entries;
    const QJsonArray items = document.object().value(QLatin1String("items")).toArray();
    for (const auto &value : items) {
        if (auto entry = ClipboardEntry::fromJson(value.toObject())) {
            entries.append(std::move(*entry));
        }
    }
    m_errorString.clear();
    return entries;
}
