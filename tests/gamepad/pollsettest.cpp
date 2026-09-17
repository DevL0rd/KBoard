#include "pollset.h"
#include "gamepadbackend.h"

#include <QElapsedTimer>
#include <QTest>

#include <array>
#include <fcntl.h>
#include <sys/eventfd.h>
#include <unistd.h>

namespace
{
struct Pipe
{
    std::array<int, 2> fds {-1, -1};

    Pipe() { QVERIFY(::pipe2(fds.data(), O_CLOEXEC | O_NONBLOCK) == 0); }
    ~Pipe()
    {
        closeRead();
        closeWrite();
    }
    Pipe(const Pipe &) = delete;
    Pipe &operator=(const Pipe &) = delete;

    int readEnd() const { return fds[0]; }
    void send() const { QCOMPARE(::write(fds[1], "x", 1), 1); }
    void closeRead()
    {
        if (fds[0] >= 0) {
            ::close(fds[0]);
        }
        fds[0] = -1;
    }
    void closeWrite()
    {
        if (fds[1] >= 0) {
            ::close(fds[1]);
        }
        fds[1] = -1;
    }
};
}

class PollSetTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void timesOutWhenQuiet()
    {
        const int wake = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
        Pipe hotplug;
        Pipe device;
        PollSet set;
        set.setControlDescriptors(wake, hotplug.readEnd());
        set.setDeviceDescriptors({device.readEnd()});
        QElapsedTimer clock;
        clock.start();
        const PollSet::Result result = set.wait(40);
        QVERIFY(clock.elapsed() >= 35);
        QVERIFY(result.ok);
        QVERIFY(!result.wake && !result.hotplug && !result.device && !result.invalid);
        ::close(wake);
    }

    void reportsEachSource()
    {
        const int wake = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
        Pipe hotplug;
        Pipe first;
        Pipe second;
        PollSet set;
        set.setControlDescriptors(wake, hotplug.readEnd());
        set.setDeviceDescriptors({first.readEnd(), second.readEnd()});
        QCOMPARE(set.deviceDescriptors(), (QList<int> {first.readEnd(), second.readEnd()}));

        const quint64 one = 1;
        QCOMPARE(::write(wake, &one, sizeof(one)), ssize_t(sizeof(one)));
        PollSet::Result result = set.wait(-1);
        QVERIFY(result.wake && !result.hotplug && !result.device);
        quint64 counter = 0;
        QCOMPARE(::read(wake, &counter, sizeof(counter)), ssize_t(sizeof(counter)));

        hotplug.send();
        result = set.wait(-1);
        QVERIFY(!result.wake && result.hotplug && !result.device);

        second.send();
        result = set.wait(0);
        QVERIFY(result.hotplug && result.device && !result.invalid);
        ::close(wake);
    }

    void closedDeviceIsInvalid()
    {
        const int wake = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
        Pipe hotplug;
        Pipe device;
        const int stale = device.readEnd();
        device.closeRead();
        PollSet set;
        set.setControlDescriptors(wake, hotplug.readEnd());
        set.setDeviceDescriptors({stale});
        const PollSet::Result result = set.wait(1000);
        QVERIFY(result.ok);
        QVERIFY(result.invalid);
        ::close(wake);
    }

    void deadlineTimeouts()
    {
        QCOMPARE(GamepadBackend::nextTimeoutMs(QDeadlineTimer(QDeadlineTimer::Forever)), -1);
        QCOMPARE(GamepadBackend::nextTimeoutMs(QDeadlineTimer(0)), -1);
        const int timeout = GamepadBackend::nextTimeoutMs(QDeadlineTimer(200));
        QVERIFY(timeout > 150 && timeout <= 201);
    }
};

QTEST_GUILESS_MAIN(PollSetTest)
#include "pollsettest.moc"
