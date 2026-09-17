#include "gamepad.h"

#include "gamepadbackend.h"
#include "gamepadbuttons.h"
#include "kboardpaths.h"
#include "kboardsettings.h"

#include <QCoreApplication>
#include <QFileInfo>

#include <SDL3/SDL_gamepad.h>

#include <algorithm>
#include <cmath>
#include <numbers>

namespace
{
constexpr double DegreesPerHalfTurn = 180.0;
constexpr double FullTurnDegrees = 360.0;
}

Gamepad::Gamepad(Mode mode, QObject *parent)
    : QObject(parent)
    , m_settings(KBoardSettings::self())
    , m_mode(mode)
{
    connect(&m_repeater, &NavigationRepeater::navigate, this, &Gamepad::navigate);
    connect(&m_buttons, &ButtonSemantics::pressed, this, &Gamepad::buttonPressed);
    connect(&m_buttons, &ButtonSemantics::released, this, &Gamepad::buttonReleased);
    connect(&m_buttons, &ButtonSemantics::tapped, this, &Gamepad::buttonTapped);
    connect(&m_buttons, &ButtonSemantics::chordActivated, this, &Gamepad::chordActivated);
    m_repeater.setSpeed(m_settings->controllerStickSpeed());
    applyChord();
    connectSettings();
    if (m_mode == Mode::WithBackend) {
        connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, this, &Gamepad::stopBackend);
        applyEnabled();
    }
}

Gamepad::~Gamepad()
{
    stopBackend();
}

Gamepad *Gamepad::instance()
{
    static QPointer<Gamepad> gamepad;
    if (!gamepad) {
        gamepad = new Gamepad(Mode::WithBackend, QCoreApplication::instance());
    }
    return gamepad;
}

Gamepad *Gamepad::create(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)
    Gamepad *gamepad = instance();
    QJSEngine::setObjectOwnership(gamepad, QJSEngine::CppOwnership);
    return gamepad;
}

void Gamepad::connectSettings()
{
    connect(m_settings, &KBoardSettings::controllerEnabledChanged, this, &Gamepad::applyEnabled);
    connect(m_settings, &KBoardSettings::controllerOpenChordChanged, this, &Gamepad::applyChord);
    connect(m_settings, &KBoardSettings::controllerRumbleChanged, this, &Gamepad::activeDeviceChanged);
    connect(m_settings, &KBoardSettings::controllerStickSpeedChanged, this,
        [this] { m_repeater.setSpeed(m_settings->controllerStickSpeed()); });
    connect(m_settings, &KBoardSettings::controllerDeadzoneChanged, this, [this] {
        Q_EMIT deadzoneChanged();
        updateLeftStickNavigation();
    });
    connect(m_settings, &KBoardSettings::controllerRadialModeChanged, this, [this] {
        Q_EMIT radialModeChanged();
        updateLeftStickNavigation();
    });
}

bool Gamepad::enabled() const
{
    return m_settings->controllerEnabled();
}

QString Gamepad::name() const
{
    const DeviceState *state = m_devices.active();
    return state ? state->info.name : QString();
}

QString Gamepad::controllerType() const
{
    const DeviceState *state = m_devices.active();
    return state ? state->info.controllerType : QStringLiteral("generic");
}

bool Gamepad::rumbleSupported() const
{
    const DeviceState *state = m_devices.active();
    return state && state->info.rumble;
}

QStringList Gamepad::heldButtons() const
{
    const DeviceState *state = m_devices.active();
    if (!state) {
        return {};
    }
    QStringList held(state->held.cbegin(), state->held.cend());
    std::sort(held.begin(), held.end());
    return held;
}

double Gamepad::axisValue(int axis) const
{
    const DeviceState *state = m_devices.active();
    return state ? state->axes.at(axis) : 0.0;
}

QPointF Gamepad::stickValue(int xAxis, int yAxis) const
{
    return {axisValue(xAxis), axisValue(yAxis)};
}

QPointF Gamepad::leftStick() const
{
    return stickValue(SDL_GAMEPAD_AXIS_LEFTX, SDL_GAMEPAD_AXIS_LEFTY);
}

QPointF Gamepad::rightStick() const
{
    return stickValue(SDL_GAMEPAD_AXIS_RIGHTX, SDL_GAMEPAD_AXIS_RIGHTY);
}

double Gamepad::leftTrigger() const
{
    return axisValue(SDL_GAMEPAD_AXIS_LEFT_TRIGGER);
}

double Gamepad::rightTrigger() const
{
    return axisValue(SDL_GAMEPAD_AXIS_RIGHT_TRIGGER);
}

double Gamepad::deadzone() const
{
    return m_settings->controllerDeadzone();
}

bool Gamepad::radialMode() const
{
    return m_settings->controllerRadialMode();
}

QStringList Gamepad::buttonNames() const
{
    return GamepadButtons::names();
}

QStringList Gamepad::controllerTypes() const
{
    return GamepadButtons::controllerTypes();
}

QString Gamepad::glyph(const QString &button) const
{
    return GamepadButtons::glyph(controllerType(), button);
}

QString Gamepad::glyphFor(const QString &type, const QString &button) const
{
    return GamepadButtons::glyph(type, button);
}

bool Gamepad::isHeld(const QString &button) const
{
    const DeviceState *state = m_devices.active();
    return state && state->held.contains(GamepadButtons::canonicalName(button));
}

bool Gamepad::rumble(double strength, int durationMs)
{
    if (!m_settings->controllerRumble() || !rumbleSupported() || !m_backend) {
        return false;
    }
    m_backend->rumble(m_devices.activeId(), strength, durationMs);
    return true;
}

double Gamepad::stickAngle(QPointF stick)
{
    if (stick.isNull()) {
        return 0.0;
    }
    const double degrees = std::atan2(stick.x(), -stick.y()) * DegreesPerHalfTurn / std::numbers::pi;
    return std::fmod(degrees + FullTurnDegrees, FullTurnDegrees);
}

double Gamepad::stickMagnitude(QPointF stick)
{
    return std::min(1.0, std::hypot(stick.x(), stick.y()));
}

void Gamepad::applyEnabled()
{
    Q_EMIT enabledChanged();
    if (m_mode != Mode::WithBackend) {
        return;
    }
    if (m_settings->controllerEnabled()) {
        startBackend();
    } else {
        stopBackend();
    }
}

void Gamepad::startBackend()
{
    if (m_backend) {
        return;
    }
    QStringList mappings = {KBoardPaths::dataFile(QStringLiteral("gamepad/gamecontrollerdb.txt"))};
    const QString userMappings = KBoardPaths::userDataFile(QStringLiteral("gamecontrollerdb.txt"));
    if (QFileInfo::exists(userMappings)) {
        mappings.append(userMappings);
    }
    m_warnings.clear();
    Q_EMIT warningsChanged();
    setErrorString({});

    m_backend = new GamepadBackend(mappings, this);
    m_backendContext = new QObject(this);
    connect(m_backend, &GamepadBackend::initialized, m_backendContext, [this] { setAvailable(true); });
    connect(m_backend, &GamepadBackend::failed, m_backendContext, [this](const QString &error) {
        setAvailable(false);
        setErrorString(error);
    });
    connect(m_backend, &GamepadBackend::warning, m_backendContext, [this](const QString &message) { addWarning(message); });
    connect(m_backend, &GamepadBackend::deviceAdded, m_backendContext, [this](const GamepadDevice &device) { handleDeviceAdded(device); });
    connect(m_backend, &GamepadBackend::deviceRemoved, m_backendContext, [this](quint32 id) { handleDeviceRemoved(id); });
    connect(m_backend, &GamepadBackend::buttonChanged, m_backendContext,
        [this](quint32 id, const QString &button, bool down, qint64 ms) { handleButton(id, button, down, ms); });
    connect(m_backend, &GamepadBackend::axisChanged, m_backendContext,
        [this](quint32 id, int axis, double value, qint64 ms) { handleAxis(id, axis, value, ms); });
    m_backend->start();
}

void Gamepad::stopBackend()
{
    if (!m_backend) {
        return;
    }
    delete m_backendContext;
    m_backend->requestStop();
    m_backend->wait();
    delete m_backend;
    clearDevices();
    setAvailable(false);
}

void Gamepad::clearDevices()
{
    releaseActiveState();
    if (m_devices.isEmpty()) {
        return;
    }
    m_devices.clear();
    Q_EMIT activeDeviceChanged();
    Q_EMIT devicesChanged();
    Q_EMIT heldButtonsChanged();
    emitAllAxesChanged();
}

void Gamepad::setAvailable(bool available)
{
    if (m_available == available) {
        return;
    }
    m_available = available;
    Q_EMIT availableChanged();
}

void Gamepad::setErrorString(const QString &error)
{
    if (m_errorString == error) {
        return;
    }
    m_errorString = error;
    if (!error.isEmpty()) {
        qWarning("KBoard gamepad: %s", qPrintable(error));
    }
    Q_EMIT errorStringChanged();
}

void Gamepad::addWarning(const QString &warning)
{
    qWarning("KBoard gamepad: %s", qPrintable(warning));
    m_warnings.append(warning);
    Q_EMIT warningsChanged();
}

void Gamepad::applyChord()
{
    const DeviceState *state = m_devices.active();
    if (!m_buttons.setChord(m_settings->controllerOpenChord(), state ? state->held : QSet<QString>())) {
        qWarning("KBoard gamepad: %s", qPrintable(m_buttons.chordError()));
    }
    Q_EMIT chordChanged();
}
