#pragma once

#include <QJsonDocument>
#include <QString>

namespace JsonFile
{
QJsonDocument read(const QString &path, QString *error);
void write(const QString &path, const QJsonDocument &document);
}
