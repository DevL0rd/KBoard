#include "hotplugmonitor.h"

#include <libudev.h>

#include <cerrno>
#include <cstring>
#include <sys/eventfd.h>
#include <unistd.h>

HotplugMonitor::HotplugMonitor()
    : m_wakeFd(eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK))
{
    if (m_wakeFd < 0) {
        m_error = QStringLiteral("Could not create the controller wake-up descriptor: %1").arg(QString::fromLocal8Bit(strerror(errno)));
        return;
    }
    if (!openMonitor()) {
        m_error = QStringLiteral("Could not watch udev for controller hotplug");
    }
}

HotplugMonitor::~HotplugMonitor()
{
    if (m_monitor) {
        udev_monitor_unref(m_monitor);
    }
    if (m_udev) {
        udev_unref(m_udev);
    }
    if (m_wakeFd >= 0) {
        ::close(m_wakeFd);
    }
}

bool HotplugMonitor::openMonitor()
{
    m_udev = udev_new();
    if (!m_udev) {
        return false;
    }
    m_monitor = udev_monitor_new_from_netlink(m_udev, "udev");
    if (!m_monitor) {
        return false;
    }
    for (const char *subsystem : {"input", "hidraw"}) {
        if (udev_monitor_filter_add_match_subsystem_devtype(m_monitor, subsystem, nullptr) < 0) {
            return false;
        }
    }
    return udev_monitor_enable_receiving(m_monitor) >= 0;
}

bool HotplugMonitor::isValid() const
{
    return m_error.isEmpty();
}

QString HotplugMonitor::errorString() const
{
    return m_error;
}

int HotplugMonitor::monitorFd() const
{
    return m_monitor ? udev_monitor_get_fd(m_monitor) : -1;
}

int HotplugMonitor::wakeFd() const
{
    return m_wakeFd;
}

void HotplugMonitor::wake()
{
    if (m_wakeFd < 0) {
        return;
    }
    const quint64 one = 1;
    [[maybe_unused]] const auto written = ::write(m_wakeFd, &one, sizeof(one));
}

void HotplugMonitor::clearWake()
{
    quint64 counter = 0;
    [[maybe_unused]] const auto readBytes = ::read(m_wakeFd, &counter, sizeof(counter));
}

void HotplugMonitor::discardPending()
{
    if (!m_monitor) {
        return;
    }
    while (udev_device *device = udev_monitor_receive_device(m_monitor)) {
        udev_device_unref(device);
    }
}
