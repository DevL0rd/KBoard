#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QQuickItem>

class FocusDriver : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    explicit FocusDriver(QObject *parent = nullptr);

    Q_INVOKABLE void sendKey(QQuickItem *anyItem, int key, int modifiers = 0);
    Q_INVOKABLE bool moveFocus(QQuickItem *anyItem, bool forward);
};
