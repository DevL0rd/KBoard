#include "pollset.h"

#include <cerrno>
#include <cstring>
#include <poll.h>
#include <vector>

namespace
{

constexpr unsigned ReadableEvents = unsigned {POLLIN} | unsigned {POLLERR} | unsigned {POLLHUP} | unsigned {POLLNVAL};
constexpr unsigned BrokenEvents = unsigned {POLLNVAL} | unsigned {POLLHUP} | unsigned {POLLERR};

bool hasEvent(const pollfd &entry, unsigned events)
{
    return (static_cast<unsigned>(entry.revents) & events) != 0;
}

}

void PollSet::setControlDescriptors(int wakeFd, int hotplugFd)
{
    m_wakeFd = wakeFd;
    m_hotplugFd = hotplugFd;
}

void PollSet::setDeviceDescriptors(const QList<int> &descriptors)
{
    m_devices = descriptors;
}

QList<int> PollSet::deviceDescriptors() const
{
    return m_devices;
}

PollSet::Result PollSet::wait(int timeoutMs) const
{
    std::vector<pollfd> fds;
    fds.reserve(m_devices.size() + 2);
    fds.push_back({m_wakeFd, POLLIN, 0});
    fds.push_back({m_hotplugFd, POLLIN, 0});
    for (int descriptor : m_devices) {
        fds.push_back({descriptor, POLLIN, 0});
    }

    Result result;
    int ready = 0;
    do {
        ready = ::poll(fds.data(), fds.size(), timeoutMs);
    } while (ready < 0 && errno == EINTR);
    if (ready < 0) {
        result.ok = false;
        result.error = QStringLiteral("Waiting for controller input failed: %1").arg(QString::fromLocal8Bit(strerror(errno)));
        return result;
    }

    result.wake = hasEvent(fds.at(0), ReadableEvents);
    result.hotplug = hasEvent(fds.at(1), ReadableEvents);
    for (size_t index = 2; index < fds.size(); ++index) {
        result.device = result.device || hasEvent(fds.at(index), ReadableEvents);
        result.invalid = result.invalid || hasEvent(fds.at(index), BrokenEvents);
    }
    return result;
}
