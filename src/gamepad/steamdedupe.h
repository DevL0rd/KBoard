#pragma once

#include <QHash>
#include <QList>
#include <QString>

class SteamDedupe
{
public:
    static constexpr quint16 ValveVendor = 0x28de;
    static constexpr quint16 SteamVirtualProduct = 0x11ff;
    static constexpr qint64 CorrelationWindowMs = 80;

    struct Device
    {
        quint32 id = 0;
        quint16 vendor = 0;
        quint16 product = 0;
        bool steamVirtual = false;
    };

    struct PressResult
    {
        bool forward = true;
        quint32 replacedPhysical = 0;
    };

    static bool isSteamVirtual(quint16 rawVendor, quint16 rawProduct, quint64 steamHandle, const QString &name);

    void addDevice(const Device &device);
    void removeDevice(quint32 id);
    void clear();

    bool isIgnored(quint32 id) const;
    quint32 physicalFor(quint32 virtualId) const;
    quint32 virtualFor(quint32 physicalId) const;
    bool hasUnpairedVirtual() const;
    QString reasonIgnored(quint32 id) const;

    PressResult press(quint32 id, const QString &button, qint64 timestampMs);

private:
    struct RecentPress
    {
        QString button;
        qint64 timestampMs = 0;
    };

    const Device *findDevice(quint32 id) const;
    bool isFree(const Device &device) const;
    const Device *correlatedPartner(const Device &self, const QString &button, qint64 timestampMs) const;
    bool knowsPhysicalIdentity(const Device &virtualDevice) const;
    void pairByIdentity();
    bool isPairedPhysical(quint32 id) const;
    bool isPairedVirtual(quint32 id) const;

    QList<Device> m_devices;
    QHash<quint32, quint32> m_virtualToPhysical;
    QHash<quint32, RecentPress> m_recent;
};
