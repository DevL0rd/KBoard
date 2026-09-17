#include "textexpansions.h"

QString expansionIn(const QStringList &entries, const QString &word)
{
    const QString trimmed = word.trimmed();
    if (trimmed.isEmpty()) {
        return QString();
    }
    QString caseInsensitive;
    for (const QString &entry : entries) {
        const qsizetype separator = entry.indexOf(u'=');
        if (separator <= 0) {
            continue;
        }
        const QString shortcut = entry.left(separator).trimmed();
        if (shortcut == trimmed) {
            return entry.mid(separator + 1);
        }
        if (caseInsensitive.isEmpty() && shortcut.compare(trimmed, Qt::CaseInsensitive) == 0) {
            caseInsensitive = entry.mid(separator + 1);
        }
    }
    return caseInsensitive;
}
