#pragma once

#include <QObject>
#include <QPoint>
#include <QTimer>

namespace RepeatSchedule
{
constexpr int InitialDelayMs = 350;
constexpr int FirstRepeatMs = 160;
constexpr double Acceleration = 0.8;
constexpr int FastestRepeatMs = 25;
constexpr int AbsoluteFloorMs = 8;

int delayForStep(int step, double speed);
}

namespace StickDirection
{
constexpr double ReleaseRatio = 0.75;
constexpr double AxisLockRatio = 0.7;

QPoint resolve(double x, double y, double deadzone, QPoint previous);
}

class NavigationRepeater : public QObject
{
    Q_OBJECT

public:
    enum class Source
    {
        Dpad,
        Stick,
    };

    explicit NavigationRepeater(QObject *parent = nullptr);

    void setSpeed(double speed);
    void setDirection(Source source, QPoint direction);
    QPoint direction() const;
    int step() const;
    bool isRepeating() const;
    int pendingDelay() const;
    void stop();

Q_SIGNALS:
    void navigate(int dx, int dy);

private:
    void apply();
    void fire();

    QTimer m_timer;
    QPoint m_dpad;
    QPoint m_stick;
    QPoint m_current;
    double m_speed = 1.0;
    int m_step = 0;
};
