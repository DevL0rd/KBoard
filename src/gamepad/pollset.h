#pragma once

#include <QList>
#include <QString>

class PollSet
{
public:
    struct Result
    {
        bool ok = true;
        bool wake = false;
        bool hotplug = false;
        bool device = false;
        bool invalid = false;
        QString error;
    };

    void setControlDescriptors(int wakeFd, int hotplugFd);
    void setDeviceDescriptors(const QList<int> &descriptors);
    QList<int> deviceDescriptors() const;

    Result wait(int timeoutMs) const;

private:
    int m_wakeFd = -1;
    int m_hotplugFd = -1;
    QList<int> m_devices;
};
