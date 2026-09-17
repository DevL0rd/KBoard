#pragma once

#include <QString>

struct udev;
struct udev_monitor;

class HotplugMonitor
{
public:
    HotplugMonitor();
    ~HotplugMonitor();
    HotplugMonitor(const HotplugMonitor &) = delete;
    HotplugMonitor &operator=(const HotplugMonitor &) = delete;

    bool isValid() const;
    QString errorString() const;
    int monitorFd() const;
    int wakeFd() const;

    void wake();
    void clearWake();
    void discardPending();

private:
    bool openMonitor();

    udev *m_udev = nullptr;
    udev_monitor *m_monitor = nullptr;
    int m_wakeFd = -1;
    QString m_error;
};
