#include "pollset.h"

#include <cerrno>
#include <cstring>
#include <poll.h>
#include <vector>

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

    const auto readable = [](const pollfd &entry) { return (entry.revents & (POLLIN | POLLERR | POLLHUP | POLLNVAL)) != 0; };
    result.wake = readable(fds.at(0));
    result.hotplug = readable(fds.at(1));
    for (size_t index = 2; index < fds.size(); ++index) {
        result.device = result.device || readable(fds.at(index));
        result.invalid = result.invalid || (fds.at(index).revents & (POLLNVAL | POLLHUP | POLLERR)) != 0;
    }
    return result;
}
