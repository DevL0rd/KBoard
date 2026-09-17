#pragma once

#include <QByteArray>
#include <QFile>
#include <QString>
#include <QtLogging>

inline QString fixturePath(const QString &name)
{
    return QStringLiteral(KBOARD_GIF_FIXTURES "/") + name;
}

inline QByteArray fixture(const QString &name)
{
    QFile file(fixturePath(name));
    if (!file.open(QIODevice::ReadOnly)) {
        qFatal("missing fixture %s", qPrintable(name));
    }
    return file.readAll();
}
