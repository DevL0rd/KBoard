#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QString>

class KBoardPaths : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QString dataDir READ dataDirectory CONSTANT)
    Q_PROPERTY(QString userDataDir READ userDataDirectory CONSTANT)
    Q_PROPERTY(QString cacheDir READ cacheDirectory CONSTANT)

public:
    explicit KBoardPaths(QObject *parent = nullptr);

    static QString dataDirectory();
    static QString userDataDirectory();
    static QString cacheDirectory();
    static QString dataFile(const QString &relative);
    static QString userDataFile(const QString &relative);
};
