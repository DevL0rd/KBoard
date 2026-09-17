#include "navigationrepeat.h"

#include <algorithm>
#include <cmath>

namespace RepeatSchedule
{
int delayForStep(int step, double speed)
{
    const double clampedSpeed = std::clamp(speed, 0.25, 3.0);
    const int floorMs = std::max(AbsoluteFloorMs, static_cast<int>(std::lround(FastestRepeatMs / clampedSpeed)));
    if (step <= 0) {
        return std::max(floorMs, static_cast<int>(std::lround(InitialDelayMs / clampedSpeed)));
    }
    const double interval = FirstRepeatMs * std::pow(Acceleration, step - 1) / clampedSpeed;
    return std::max(floorMs, static_cast<int>(std::lround(interval)));
}
}

namespace StickDirection
{
QPoint resolve(double x, double y, double deadzone, QPoint previous)
{
    const double ax = std::abs(x);
    const double ay = std::abs(y);
    const double release = deadzone * ReleaseRatio;

    if (previous.x() != 0 && previous.y() == 0) {
        if (ax >= release && ax >= ay * AxisLockRatio && (x > 0) == (previous.x() > 0)) {
            return previous;
        }
    } else if (previous.y() != 0 && previous.x() == 0) {
        if (ay >= release && ay >= ax * AxisLockRatio && (y > 0) == (previous.y() > 0)) {
            return previous;
        }
    }

    if (std::max(ax, ay) < deadzone) {
        return {};
    }
    if (ax >= ay) {
        return {x > 0 ? 1 : -1, 0};
    }
    return {0, y > 0 ? 1 : -1};
}
}

NavigationRepeater::NavigationRepeater(QObject *parent)
    : QObject(parent)
{
    m_timer.setSingleShot(true);
    m_timer.setTimerType(Qt::PreciseTimer);
    connect(&m_timer, &QTimer::timeout, this, [this] {
        ++m_step;
        fire();
    });
}

void NavigationRepeater::setSpeed(double speed)
{
    m_speed = speed;
}

void NavigationRepeater::setDirection(Source source, QPoint direction)
{
    if (source == Source::Dpad) {
        m_dpad = direction;
    } else {
        m_stick = direction;
    }
    apply();
}

QPoint NavigationRepeater::direction() const
{
    return m_current;
}

int NavigationRepeater::step() const
{
    return m_step;
}

bool NavigationRepeater::isRepeating() const
{
    return m_timer.isActive();
}

int NavigationRepeater::pendingDelay() const
{
    return m_timer.isActive() ? m_timer.interval() : -1;
}

void NavigationRepeater::stop()
{
    m_dpad = {};
    m_stick = {};
    m_current = {};
    m_step = 0;
    m_timer.stop();
}

void NavigationRepeater::apply()
{
    const QPoint wanted = m_dpad.isNull() ? m_stick : m_dpad;
    if (wanted == m_current) {
        return;
    }
    m_current = wanted;
    m_step = 0;
    m_timer.stop();
    if (!m_current.isNull()) {
        fire();
    }
}

void NavigationRepeater::fire()
{
    Q_EMIT navigate(m_current.x(), m_current.y());
    if (m_current.isNull()) {
        return;
    }
    m_timer.start(RepeatSchedule::delayForStep(m_step, m_speed));
}
