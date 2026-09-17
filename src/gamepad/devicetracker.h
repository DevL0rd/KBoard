#pragma once

#include "gamepaddevice.h"
#include "steamdedupe.h"

#include <QHash>
#include <QList>
#include <QSet>
#include <QVariantList>

#include <array>

struct DeviceState
{
    static constexpr int AxisCount = 6;

    GamepadDevice info;
    QSet<QString> held;
    std::array<double, AxisCount> axes {};
};

class DeviceTracker
{
public:
    struct PressRoute
    {
        bool forward = true;
        bool activeChanged = false;
    };

    void add(const GamepadDevice &device);
    void remove(quint32 id);
    void clear();

    DeviceState *find(quint32 id);
    const DeviceState *find(quint32 id) const;
    DeviceState *active();
    const DeviceState *active() const;
    quint32 activeId() const;
    void setActiveId(quint32 id);
    bool isEmpty() const;

    bool accepts(quint32 id) const;
    PressRoute routePress(quint32 id, const QString &button, qint64 timestampMs);
    quint32 preferredActive() const;
    QVariantList toVariantList() const;

private:
    QHash<quint32, DeviceState> m_devices;
    QList<quint32> m_order;
    SteamDedupe m_dedupe;
    quint32 m_activeId = 0;
};
