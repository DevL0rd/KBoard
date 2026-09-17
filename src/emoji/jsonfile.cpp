#include "jsonfile.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonParseError>
#include <QSaveFile>

using namespace Qt::Literals::StringLiterals;

namespace JsonFile
{
QJsonDocument read(const QString &path, QString *error)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        *error = u"Cannot open %1: %2"_s.arg(path, file.errorString());
        return {};
    }
    QJsonParseError parseError;
    QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        *error = u"Cannot parse %1: %2"_s.arg(path, parseError.errorString());
        return {};
    }
    return document;
}

void write(const QString &path, const QJsonDocument &document)
{
    QDir().mkpath(QFileInfo(path).absolutePath());
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning("Cannot write %s: %s", qPrintable(path), qPrintable(file.errorString()));
        return;
    }
    file.write(document.toJson(QJsonDocument::Compact));
    if (!file.commit()) {
        qWarning("Cannot save %s: %s", qPrintable(path), qPrintable(file.errorString()));
    }
}
}
