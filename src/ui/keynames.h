#pragma once

#include <QObject>
#include <QQmlEngine>

class KeyNames : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    explicit KeyNames(QObject *parent = nullptr);

    Q_INVOKABLE static uint evdev(const QString &name);
    Q_INVOKABLE static bool needsShift(const QString &text);
    Q_INVOKABLE static uint modifierMask(const QString &modifier);
};
