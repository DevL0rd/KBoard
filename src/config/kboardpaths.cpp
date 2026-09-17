#include "kboardpaths.h"

#include <QDir>
#include <QStandardPaths>

KBoardPaths::KBoardPaths(QObject *parent)
    : QObject(parent)
{
}

QString KBoardPaths::dataDirectory()
{
    if (qEnvironmentVariableIsSet("KBOARD_USE_BUILD_TREE")) {
        return QStringLiteral(KBOARD_DATA_BUILD_DIR);
    }
    return QStringLiteral(KBOARD_DATA_INSTALL_DIR);
}

QString KBoardPaths::userDataDirectory()
{
    const QString path = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/kboard");
    QDir().mkpath(path);
    return path;
}

QString KBoardPaths::cacheDirectory()
{
    const QString path = QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation) + QStringLiteral("/kboard");
    QDir().mkpath(path);
    return path;
}

QString KBoardPaths::dataFile(const QString &relative)
{
    return dataDirectory() + QLatin1Char('/') + relative;
}

QString KBoardPaths::userDataFile(const QString &relative)
{
    return userDataDirectory() + QLatin1Char('/') + relative;
}
