#pragma once

#include <QMetaType>
#include <QString>

struct GamepadDevice
{
    quint32 id = 0;
    QString name;
    QString controllerType;
    QString path;
    quint16 vendor = 0;
    quint16 product = 0;
    quint16 rawVendor = 0;
    quint16 rawProduct = 0;
    quint64 steamHandle = 0;
    bool steamVirtual = false;
    bool rumble = false;
};

Q_DECLARE_METATYPE(GamepadDevice)
