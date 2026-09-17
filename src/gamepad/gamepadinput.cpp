#include "gamepad.h"

#include "kboardsettings.h"

#include <SDL3/SDL_gamepad.h>

#include <array>

namespace
{
constexpr double TriggerPressThreshold = 0.5;
constexpr double TriggerReleaseThreshold = 0.3;

bool isTrigger(int axis)
{
    return axis == SDL_GAMEPAD_AXIS_LEFT_TRIGGER || axis == SDL_GAMEPAD_AXIS_RIGHT_TRIGGER;
}

QPoint dpadDirection(const QSet<QString> &held)
{
    const auto axis = [&held](const char *negative, const char *positive) {
        return (held.contains(QLatin1String(positive)) ? 1 : 0) - (held.contains(QLatin1String(negative)) ? 1 : 0);
    };
    return {axis("dpleft", "dpright"), axis("dpup", "dpdown")};
}

using AxisSignal = void (Gamepad::*)();

constexpr std::array<AxisSignal, DeviceState::AxisCount> axisSignals = {
    &Gamepad::leftStickChanged,
    &Gamepad::leftStickChanged,
    &Gamepad::rightStickChanged,
    &Gamepad::rightStickChanged,
    &Gamepad::leftTriggerChanged,
    &Gamepad::rightTriggerChanged,
};
}

void Gamepad::handleDeviceAdded(const GamepadDevice &device)
{
    m_devices.add(device);
    const quint32 preferred = m_devices.preferredActive();
    if (preferred != m_devices.activeId()) {
        releaseActiveState();
        m_devices.setActiveId(preferred);
        Q_EMIT activeDeviceChanged();
        emitAllAxesChanged();
    }
    Q_EMIT devicesChanged();
}

void Gamepad::handleDeviceRemoved(quint32 id)
{
    if (!m_devices.find(id)) {
        return;
    }
    const bool wasActive = id == m_devices.activeId();
    if (wasActive) {
        releaseActiveState();
    }
    m_devices.remove(id);
    if (wasActive) {
        m_devices.setActiveId(m_devices.preferredActive());
        Q_EMIT activeDeviceChanged();
        emitAllAxesChanged();
    }
    Q_EMIT devicesChanged();
}

void Gamepad::handleButton(quint32 id, const QString &button, bool down, qint64 timestampMs)
{
    if (!m_devices.accepts(id)) {
        return;
    }
    if (down) {
        pressButton(id, button, timestampMs);
    } else {
        releaseButton(id, button);
    }
}

void Gamepad::handleAxis(quint32 id, int axis, double value, qint64 timestampMs)
{
    if (!m_devices.accepts(id) || axis < 0 || axis >= DeviceState::AxisCount) {
        return;
    }
    DeviceState *state = m_devices.find(id);
    state->axes.at(axis) = value;
    if (id != m_devices.activeId()) {
        if (!isActivity(*state, axis)) {
            return;
        }
        activate(id);
    }
    emitAxisChanged(axis);
    if (axis == SDL_GAMEPAD_AXIS_LEFTX || axis == SDL_GAMEPAD_AXIS_LEFTY) {
        updateLeftStickNavigation();
    }
    if (isTrigger(axis)) {
        updateTriggerButton(id, axis, value, timestampMs);
    }
}

void Gamepad::pressButton(quint32 id, const QString &button, qint64 timestampMs)
{
    if (m_devices.find(id)->held.contains(button)) {
        return;
    }
    const DeviceTracker::PressRoute route = m_devices.routePress(id, button, timestampMs);
    if (route.activeChanged) {
        Q_EMIT activeDeviceChanged();
    }
    if (!route.forward) {
        Q_EMIT devicesChanged();
        return;
    }
    activate(id);
    m_devices.find(id)->held.insert(button);
    Q_EMIT heldButtonsChanged();
    m_buttons.press(button);
    updateDpad();
}

void Gamepad::releaseButton(quint32 id, const QString &button)
{
    if (!m_devices.find(id)->held.remove(button) || id != m_devices.activeId()) {
        return;
    }
    Q_EMIT heldButtonsChanged();
    m_buttons.release(button);
    updateDpad();
}

void Gamepad::updateTriggerButton(quint32 id, int axis, double value, qint64 timestampMs)
{
    const QString button = axis == SDL_GAMEPAD_AXIS_LEFT_TRIGGER ? QStringLiteral("lefttrigger") : QStringLiteral("righttrigger");
    const bool held = m_devices.find(id)->held.contains(button);
    if (!held && value >= TriggerPressThreshold) {
        pressButton(id, button, timestampMs);
    } else if (held && value <= TriggerReleaseThreshold) {
        releaseButton(id, button);
    }
}

void Gamepad::activate(quint32 id)
{
    if (m_devices.activeId() == id) {
        return;
    }
    releaseActiveState();
    m_devices.setActiveId(id);
    Q_EMIT activeDeviceChanged();
    Q_EMIT devicesChanged();
    emitAllAxesChanged();
}

void Gamepad::releaseActiveState()
{
    m_repeater.stop();
    DeviceState *state = m_devices.active();
    if (!state) {
        return;
    }
    const QSet<QString> held = std::exchange(state->held, {});
    state->axes.fill(0.0);
    m_buttons.releaseAll(held);
    if (!held.isEmpty()) {
        Q_EMIT heldButtonsChanged();
    }
}

void Gamepad::updateDpad()
{
    const DeviceState *state = m_devices.active();
    m_repeater.setDirection(NavigationRepeater::Source::Dpad, state ? dpadDirection(state->held) : QPoint());
}

void Gamepad::updateLeftStickNavigation()
{
    if (!connected() || radialMode()) {
        m_repeater.setDirection(NavigationRepeater::Source::Stick, {});
        return;
    }
    const QPointF stick = leftStick();
    m_repeater.setDirection(
        NavigationRepeater::Source::Stick, StickDirection::resolve(stick.x(), stick.y(), deadzone(), m_repeater.direction()));
}

void Gamepad::emitAxisChanged(int axis)
{
    Q_EMIT(this->*axisSignals.at(axis))();
}

void Gamepad::emitAllAxesChanged()
{
    for (int axis = SDL_GAMEPAD_AXIS_LEFTX; axis < DeviceState::AxisCount; axis += 2) {
        emitAxisChanged(axis);
    }
    emitAxisChanged(SDL_GAMEPAD_AXIS_RIGHT_TRIGGER);
}

bool Gamepad::isActivity(const DeviceState &state, int axis) const
{
    if (isTrigger(axis)) {
        return state.axes.at(axis) >= TriggerPressThreshold;
    }
    const int xAxis = axis - (axis % 2);
    return std::max(std::abs(state.axes.at(xAxis)), std::abs(state.axes.at(xAxis + 1))) >= deadzone();
}
