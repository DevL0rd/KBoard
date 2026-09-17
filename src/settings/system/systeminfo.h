#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QVariantList>

class SystemInfo : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QString qtVersion READ qtVersion CONSTANT)
    Q_PROPERTY(QString frameworksVersion READ frameworksVersion CONSTANT)
    Q_PROPERTY(QString displayName READ displayName CONSTANT)
    Q_PROPERTY(QString version READ version CONSTANT)
    Q_PROPERTY(QString shortDescription READ shortDescription CONSTANT)
    Q_PROPERTY(QString bugAddress READ bugAddress CONSTANT)
    Q_PROPERTY(QVariantList authors READ authors CONSTANT)

public:
    using QObject::QObject;

    static QString qtVersion();
    static QString frameworksVersion();
    static QString displayName();
    static QString version();
    static QString shortDescription();
    static QString bugAddress();
    static QVariantList authors();
};
