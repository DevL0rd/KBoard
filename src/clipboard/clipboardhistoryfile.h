#pragma once

#include "clipboardentry.h"

#include <QList>
#include <QString>

class ClipboardHistoryFile
{
public:
    explicit ClipboardHistoryFile(QString path);

    static void ensurePrivateDirectory(const QString &path);

    QString path() const;
    QString errorString() const;
    bool save(const QList<ClipboardEntry> &entries);
    std::optional<QList<ClipboardEntry>> load();

private:
    QString m_path;
    QString m_errorString;
};
