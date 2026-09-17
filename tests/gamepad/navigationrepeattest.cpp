#include "navigationrepeat.h"

#include <QElapsedTimer>
#include <QSignalSpy>
#include <QTest>

class NavigationRepeatTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void scheduleDefaults()
    {
        QCOMPARE(RepeatSchedule::delayForStep(0, 1.0), 350);
        QCOMPARE(RepeatSchedule::delayForStep(1, 1.0), 160);
        QCOMPARE(RepeatSchedule::delayForStep(2, 1.0), 128);
        QCOMPARE(RepeatSchedule::delayForStep(3, 1.0), 102);
        QCOMPARE(RepeatSchedule::delayForStep(100, 1.0), 25);
    }

    void scheduleAccelerates()
    {
        for (double speed : {0.25, 0.5, 1.0, 2.0, 3.0}) {
            int previous = RepeatSchedule::delayForStep(0, speed);
            bool reachedFloor = false;
            for (int step = 1; step < 60; ++step) {
                const int delay = RepeatSchedule::delayForStep(step, speed);
                QVERIFY(delay <= previous);
                QVERIFY(delay >= RepeatSchedule::AbsoluteFloorMs);
                reachedFloor = reachedFloor || delay == RepeatSchedule::delayForStep(1000, speed);
                previous = delay;
            }
            QVERIFY(reachedFloor);
        }
    }

    void scheduleScalesWithSpeed()
    {
        QCOMPARE(RepeatSchedule::delayForStep(0, 2.0), 175);
        QCOMPARE(RepeatSchedule::delayForStep(0, 0.5), 700);
        QCOMPARE(RepeatSchedule::delayForStep(1000, 2.0), 13);
        QCOMPARE(RepeatSchedule::delayForStep(1000, 0.5), 50);
        QCOMPARE(RepeatSchedule::delayForStep(1000, 3.0), 8);
        QCOMPARE(RepeatSchedule::delayForStep(0, 10.0), RepeatSchedule::delayForStep(0, 3.0));
        QCOMPARE(RepeatSchedule::delayForStep(0, 0.0), RepeatSchedule::delayForStep(0, 0.25));
    }

    void stickDeadzone()
    {
        QCOMPARE(StickDirection::resolve(0.2, 0.1, 0.35, {}), QPoint());
        QCOMPARE(StickDirection::resolve(0.5, 0.1, 0.35, {}), QPoint(1, 0));
        QCOMPARE(StickDirection::resolve(-0.5, 0.2, 0.35, {}), QPoint(-1, 0));
        QCOMPARE(StickDirection::resolve(0.1, -0.9, 0.35, {}), QPoint(0, -1));
        QCOMPARE(StickDirection::resolve(0.3, 0.6, 0.35, {}), QPoint(0, 1));
    }

    void stickHysteresis()
    {
        QCOMPARE(StickDirection::resolve(0.3, 0.0, 0.35, QPoint(1, 0)), QPoint(1, 0));
        QCOMPARE(StickDirection::resolve(0.25, 0.0, 0.35, QPoint(1, 0)), QPoint());
        QCOMPARE(StickDirection::resolve(0.6, 0.7, 0.35, QPoint(1, 0)), QPoint(1, 0));
        QCOMPARE(StickDirection::resolve(0.4, 0.9, 0.35, QPoint(1, 0)), QPoint(0, 1));
        QCOMPARE(StickDirection::resolve(-0.6, 0.0, 0.35, QPoint(1, 0)), QPoint(-1, 0));
    }

    void repeaterFiresImmediatelyThenRepeats()
    {
        NavigationRepeater repeater;
        repeater.setSpeed(3.0);
        QSignalSpy spy(&repeater, &NavigationRepeater::navigate);
        QElapsedTimer clock;
        clock.start();
        repeater.setDirection(NavigationRepeater::Source::Stick, QPoint(1, 0));
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.at(0).at(0).toInt(), 1);
        QCOMPARE(spy.at(0).at(1).toInt(), 0);
        QCOMPARE(repeater.pendingDelay(), RepeatSchedule::delayForStep(0, 3.0));

        QTRY_COMPARE_WITH_TIMEOUT(spy.count(), 2, 2000);
        QVERIFY(clock.elapsed() >= RepeatSchedule::delayForStep(0, 3.0) - 2);
        QCOMPARE(repeater.pendingDelay(), RepeatSchedule::delayForStep(1, 3.0));

        QTRY_VERIFY_WITH_TIMEOUT(spy.count() >= 12, 3000);
        QCOMPARE(repeater.pendingDelay(), RepeatSchedule::delayForStep(repeater.step(), 3.0));
        QVERIFY(repeater.pendingDelay() < RepeatSchedule::delayForStep(1, 3.0));

        repeater.setDirection(NavigationRepeater::Source::Stick, QPoint());
        QVERIFY(!repeater.isRepeating());
        const int count = spy.count();
        QTest::qWait(150);
        QCOMPARE(spy.count(), count);
    }

    void dpadOverridesStick()
    {
        NavigationRepeater repeater;
        QSignalSpy spy(&repeater, &NavigationRepeater::navigate);
        repeater.setDirection(NavigationRepeater::Source::Stick, QPoint(1, 0));
        repeater.setDirection(NavigationRepeater::Source::Dpad, QPoint(0, -1));
        QCOMPARE(spy.count(), 2);
        QCOMPARE(spy.at(1).at(1).toInt(), -1);
        QCOMPARE(repeater.step(), 0);
        repeater.setDirection(NavigationRepeater::Source::Stick, QPoint(-1, 0));
        QCOMPARE(spy.count(), 2);
        repeater.setDirection(NavigationRepeater::Source::Dpad, QPoint());
        QCOMPARE(spy.count(), 3);
        QCOMPARE(spy.at(2).at(0).toInt(), -1);
        repeater.stop();
        QVERIFY(!repeater.isRepeating());
    }

    void sameDirectionDoesNotRestart()
    {
        NavigationRepeater repeater;
        QSignalSpy spy(&repeater, &NavigationRepeater::navigate);
        repeater.setDirection(NavigationRepeater::Source::Stick, QPoint(0, 1));
        repeater.setDirection(NavigationRepeater::Source::Stick, QPoint(0, 1));
        QCOMPARE(spy.count(), 1);
    }
};

QTEST_GUILESS_MAIN(NavigationRepeatTest)
#include "navigationrepeattest.moc"
