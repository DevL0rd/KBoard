#include "devicetracker.h"

#include <ranges>

void DeviceTracker::add(const GamepadDevice &device)
{
    DeviceState state;
    state.info = device;
    m_devices.insert(device.id, state);
    m_order.removeAll(device.id);
    m_order.append(device.id);
    m_dedupe.addDevice({device.id, device.vendor, device.product, device.steamVirtual});
}

void DeviceTracker::remove(quint32 id)
{
    m_devices.remove(id);
    m_order.removeAll(id);
    m_dedupe.removeDevice(id);
    if (m_activeId == id) {
        m_activeId = 0;
    }
}

void DeviceTracker::clear()
{
    m_devices.clear();
    m_order.clear();
    m_dedupe.clear();
    m_activeId = 0;
}

DeviceState *DeviceTracker::find(quint32 id)
{
    const auto it = m_devices.find(id);
    return it == m_devices.end() ? nullptr : &it.value();
}

const DeviceState *DeviceTracker::find(quint32 id) const
{
    const auto it = m_devices.constFind(id);
    return it == m_devices.cend() ? nullptr : &it.value();
}

DeviceState *DeviceTracker::active()
{
    return find(m_activeId);
}

const DeviceState *DeviceTracker::active() const
{
    return find(m_activeId);
}

quint32 DeviceTracker::activeId() const
{
    return active() ? m_activeId : 0;
}

void DeviceTracker::setActiveId(quint32 id)
{
    m_activeId = id;
}

bool DeviceTracker::isEmpty() const
{
    return m_devices.isEmpty();
}

bool DeviceTracker::accepts(quint32 id) const
{
    return m_devices.contains(id) && !m_dedupe.isIgnored(id);
}

DeviceTracker::PressRoute DeviceTracker::routePress(quint32 id, const QString &button, qint64 timestampMs)
{
    PressRoute route;
    const SteamDedupe::PressResult verdict = m_dedupe.press(id, button, timestampMs);
    route.forward = verdict.forward;
    DeviceState *physical = find(verdict.replacedPhysical);
    DeviceState *takingOver = find(id);
    if (!physical || !takingOver) {
        return route;
    }
    takingOver->held.unite(physical->held);
    takingOver->held.insert(button);
    physical->held.clear();
    if (m_activeId == verdict.replacedPhysical) {
        m_activeId = id;
        route.activeChanged = true;
    }
    return route;
}

quint32 DeviceTracker::preferredActive() const
{
    if (active() && !m_dedupe.isIgnored(m_activeId)) {
        return m_activeId;
    }
    if (m_activeId != 0 && m_dedupe.virtualFor(m_activeId) != 0) {
        return m_dedupe.virtualFor(m_activeId);
    }
    for (quint32 id : std::views::reverse(m_order)) {
        if (!m_dedupe.isIgnored(id)) {
            return id;
        }
    }
    return 0;
}

QVariantList DeviceTracker::toVariantList() const
{
    QVariantList list;
    list.reserve(m_order.size());
    for (quint32 id : m_order) {
        const GamepadDevice &info = m_devices.constFind(id)->info;
        list.append(QVariantMap {
            {QStringLiteral("id"), static_cast<int>(id)},
            {QStringLiteral("name"), info.name},
            {QStringLiteral("controllerType"), info.controllerType},
            {QStringLiteral("path"), info.path},
            {QStringLiteral("vendor"), static_cast<int>(info.vendor)},
            {QStringLiteral("product"), static_cast<int>(info.product)},
            {QStringLiteral("steamVirtual"), info.steamVirtual},
            {QStringLiteral("rumble"), info.rumble},
            {QStringLiteral("ignored"), m_dedupe.isIgnored(id)},
            {QStringLiteral("ignoredReason"), m_dedupe.reasonIgnored(id)},
            {QStringLiteral("active"), id == m_activeId},
        });
    }
    return list;
}
