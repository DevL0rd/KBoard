#include "steamdedupe.h"

#include <algorithm>
#include <cstdlib>

bool SteamDedupe::isSteamVirtual(quint16 rawVendor, quint16 rawProduct, quint64 steamHandle, const QString &name)
{
    if (steamHandle != 0) {
        return true;
    }
    if (rawVendor == ValveVendor && rawProduct == SteamVirtualProduct) {
        return true;
    }
    return name.startsWith(QLatin1String("Steam Virtual Gamepad"));
}

void SteamDedupe::addDevice(const Device &device)
{
    removeDevice(device.id);
    m_devices.append(device);
    pairByIdentity();
}

void SteamDedupe::removeDevice(quint32 id)
{
    m_recent.remove(id);
    m_virtualToPhysical.remove(id);
    for (auto it = m_virtualToPhysical.begin(); it != m_virtualToPhysical.end();) {
        if (it.value() == id) {
            it = m_virtualToPhysical.erase(it);
        } else {
            ++it;
        }
    }
    m_devices.removeIf([id](const Device &device) { return device.id == id; });
    pairByIdentity();
}

void SteamDedupe::clear()
{
    m_devices.clear();
    m_virtualToPhysical.clear();
    m_recent.clear();
}

bool SteamDedupe::isIgnored(quint32 id) const
{
    return isPairedPhysical(id);
}

quint32 SteamDedupe::physicalFor(quint32 virtualId) const
{
    return m_virtualToPhysical.value(virtualId, 0);
}

quint32 SteamDedupe::virtualFor(quint32 physicalId) const
{
    for (auto it = m_virtualToPhysical.cbegin(); it != m_virtualToPhysical.cend(); ++it) {
        if (it.value() == physicalId) {
            return it.key();
        }
    }
    return 0;
}

bool SteamDedupe::hasUnpairedVirtual() const
{
    for (const Device &device : m_devices) {
        if (device.steamVirtual && !isPairedVirtual(device.id)) {
            return true;
        }
    }
    return false;
}

QString SteamDedupe::reasonIgnored(quint32 id) const
{
    const quint32 virtualId = virtualFor(id);
    if (virtualId == 0) {
        return {};
    }
    return QStringLiteral("Steam Input is driving this controller through Steam Virtual Gamepad %1").arg(virtualId);
}

SteamDedupe::PressResult SteamDedupe::press(quint32 id, const QString &button, qint64 timestampMs)
{
    PressResult result;
    if (isPairedPhysical(id)) {
        result.forward = false;
        return result;
    }
    const Device *self = findDevice(id);
    if (!self || !isFree(*self)) {
        return result;
    }
    const Device *partner = correlatedPartner(*self, button, timestampMs);
    if (!partner) {
        m_recent.insert(id, RecentPress {button, timestampMs});
        return result;
    }
    const quint32 virtualId = self->steamVirtual ? self->id : partner->id;
    const quint32 physicalId = self->steamVirtual ? partner->id : self->id;
    m_virtualToPhysical.insert(virtualId, physicalId);
    m_recent.remove(virtualId);
    m_recent.remove(physicalId);
    result.forward = false;
    result.replacedPhysical = self->steamVirtual ? physicalId : 0;
    return result;
}

const SteamDedupe::Device *SteamDedupe::findDevice(quint32 id) const
{
    const auto it = std::find_if(m_devices.cbegin(), m_devices.cend(), [id](const Device &device) { return device.id == id; });
    return it == m_devices.cend() ? nullptr : &*it;
}

bool SteamDedupe::isFree(const Device &device) const
{
    return device.steamVirtual ? !isPairedVirtual(device.id) : !isPairedPhysical(device.id);
}

const SteamDedupe::Device *SteamDedupe::correlatedPartner(const Device &self, const QString &button, qint64 timestampMs) const
{
    for (const Device &other : m_devices) {
        if (other.steamVirtual == self.steamVirtual || !isFree(other)) {
            continue;
        }
        const auto recent = m_recent.constFind(other.id);
        if (recent != m_recent.cend() && recent->button == button && std::llabs(timestampMs - recent->timestampMs) <= CorrelationWindowMs) {
            return &other;
        }
    }
    return nullptr;
}

bool SteamDedupe::knowsPhysicalIdentity(const Device &virtualDevice) const
{
    const bool unmappedVirtual = virtualDevice.vendor == ValveVendor && virtualDevice.product == SteamVirtualProduct;
    const bool unknown = virtualDevice.vendor == 0 && virtualDevice.product == 0;
    return !unmappedVirtual && !unknown;
}

void SteamDedupe::pairByIdentity()
{
    for (const Device &virtualDevice : std::as_const(m_devices)) {
        if (!virtualDevice.steamVirtual || isPairedVirtual(virtualDevice.id) || !knowsPhysicalIdentity(virtualDevice)) {
            continue;
        }
        for (const Device &physical : std::as_const(m_devices)) {
            if (physical.steamVirtual || isPairedPhysical(physical.id)) {
                continue;
            }
            if (physical.vendor == virtualDevice.vendor && physical.product == virtualDevice.product) {
                m_virtualToPhysical.insert(virtualDevice.id, physical.id);
                break;
            }
        }
    }
}

bool SteamDedupe::isPairedPhysical(quint32 id) const
{
    for (auto it = m_virtualToPhysical.cbegin(); it != m_virtualToPhysical.cend(); ++it) {
        if (it.value() == id) {
            return true;
        }
    }
    return false;
}

bool SteamDedupe::isPairedVirtual(quint32 id) const
{
    return m_virtualToPhysical.contains(id);
}
