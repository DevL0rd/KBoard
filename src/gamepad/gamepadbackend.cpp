#include "gamepadbackend.h"

#include "gamepadbuttons.h"
#include "steamdedupe.h"

#include <SDL3/SDL.h>

#include <algorithm>

namespace
{
constexpr int AxisKeyShift = 8;
constexpr quint64 AxisKeyMask = 0xff;
constexpr double RumbleLevelMax = 0xffff;
constexpr double AxisScale = 32767.0;
constexpr int RumbleSettleMs = 20;

qint64 nsToMs(Uint64 ns)
{
    return static_cast<qint64>(ns / SDL_NS_PER_MS);
}

double normalizeAxis(int axis, Sint16 value)
{
    const bool trigger = axis == SDL_GAMEPAD_AXIS_LEFT_TRIGGER || axis == SDL_GAMEPAD_AXIS_RIGHT_TRIGGER;
    return std::clamp(value / AxisScale, trigger ? 0.0 : -1.0, 1.0);
}

QString sdlError()
{
    return QString::fromUtf8(SDL_GetError());
}

GamepadDevice describeGamepad(quint32 id, SDL_Gamepad *pad)
{
    GamepadDevice device;
    device.id = id;
    device.name = QString::fromUtf8(SDL_GetGamepadName(pad));
    device.path = QString::fromUtf8(SDL_GetGamepadPath(pad));
    device.vendor = SDL_GetGamepadVendor(pad);
    device.product = SDL_GetGamepadProduct(pad);
    SDL_GetJoystickGUIDInfo(SDL_GetGamepadGUIDForID(id), &device.rawVendor, &device.rawProduct, nullptr, nullptr);
    device.steamHandle = SDL_GetGamepadSteamHandle(pad);
    device.steamVirtual = SteamDedupe::isSteamVirtual(device.rawVendor, device.rawProduct, device.steamHandle, device.name);
    device.controllerType = GamepadButtons::typeForSdl(SDL_GetGamepadType(pad), device.vendor, device.product);
    device.rumble = SDL_GetBooleanProperty(SDL_GetGamepadProperties(pad), SDL_PROP_GAMEPAD_CAP_RUMBLE_BOOLEAN, false);
    return device;
}
}

GamepadBackend::GamepadBackend(const QStringList &mappingFiles, QObject *parent)
    : QThread(parent)
    , m_mappingFiles(mappingFiles)
{
    qRegisterMetaType<GamepadDevice>();
    setObjectName(QStringLiteral("KBoardGamepad"));
    m_pollSet.setControlDescriptors(m_hotplug.wakeFd(), m_hotplug.monitorFd());
}

GamepadBackend::~GamepadBackend()
{
    requestStop();
    wait();
}

void GamepadBackend::requestStop()
{
    m_stopRequested = true;
    m_hotplug.wake();
}

void GamepadBackend::rumble(quint32 deviceId, double strength, int durationMs)
{
    {
        const QMutexLocker locker(&m_rumbleMutex);
        m_rumbleRequests.append({deviceId, static_cast<quint16>(std::clamp(strength, 0.0, 1.0) * RumbleLevelMax),
            static_cast<quint32>(std::max(0, durationMs))});
    }
    m_hotplug.wake();
}

int GamepadBackend::nextTimeoutMs(const QDeadlineTimer &deadline)
{
    if (deadline.isForever() || deadline.hasExpired()) {
        return -1;
    }
    return static_cast<int>(deadline.remainingTime()) + 1;
}

void GamepadBackend::run()
{
    if (!initialize()) {
        return;
    }
    eventLoop();
    shutdown();
}

bool GamepadBackend::initialize()
{
    if (!m_hotplug.isValid()) {
        Q_EMIT failed(m_hotplug.errorString());
        return false;
    }
    m_baseline = DescriptorScanner::snapshot();
    SDL_SetHint(SDL_HINT_NO_SIGNAL_HANDLERS, "1");
    SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
    if (!SDL_Init(SDL_INIT_GAMEPAD)) {
        Q_EMIT failed(QStringLiteral("SDL could not start the gamepad subsystem: %1").arg(sdlError()));
        return false;
    }
    loadMappings();
    m_rumbleDeadline = QDeadlineTimer(QDeadlineTimer::Forever);
    Q_EMIT initialized();
    return true;
}

void GamepadBackend::loadMappings()
{
    for (const QString &file : std::as_const(m_mappingFiles)) {
        if (SDL_AddGamepadMappingsFromFile(file.toLocal8Bit().constData()) < 0) {
            Q_EMIT warning(QStringLiteral("Could not load controller mappings from %1: %2").arg(file, sdlError()));
        }
    }
}

void GamepadBackend::eventLoop()
{
    while (!m_stopRequested.loadRelaxed()) {
        applyRumbleRequests();
        dispatchQueued();
        if (m_descriptorsStale) {
            refreshDescriptors();
        }
        const PollSet::Result result = m_pollSet.wait(nextTimeoutMs(m_rumbleDeadline));
        if (!result.ok) {
            Q_EMIT failed(result.error);
            return;
        }
        if (result.wake) {
            m_hotplug.clearWake();
        }
        if (result.hotplug) {
            m_hotplug.discardPending();
        }
        m_descriptorsStale = m_descriptorsStale || result.hotplug || result.invalid;
    }
}

void GamepadBackend::refreshDescriptors()
{
    const QSet<quint64> uevents = DescriptorScanner::ueventSocketInodes();
    m_pollSet.setDeviceDescriptors(DescriptorScanner::inputDescriptors(m_baseline, DescriptorScanner::snapshot(), uevents));
    m_descriptorsStale = false;
}

void GamepadBackend::shutdown()
{
    for (SDL_Gamepad *pad : std::as_const(m_pads)) {
        SDL_CloseGamepad(pad);
    }
    m_pads.clear();
    m_pendingAxes.clear();
    SDL_Quit();
}

void GamepadBackend::dispatchQueued()
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        processEvent(event);
    }
    flushAxes();
}

void GamepadBackend::processEvent(const SDL_Event &event)
{
    switch (event.type) {
    case SDL_EVENT_GAMEPAD_ADDED:
        openGamepad(event.gdevice.which);
        break;
    case SDL_EVENT_GAMEPAD_REMOVED:
        closeGamepad(event.gdevice.which);
        break;
    case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
    case SDL_EVENT_GAMEPAD_BUTTON_UP:
        emitButton(event);
        break;
    case SDL_EVENT_GAMEPAD_AXIS_MOTION:
        queueAxis(event);
        break;
    case SDL_EVENT_JOYSTICK_ADDED:
    case SDL_EVENT_JOYSTICK_REMOVED:
        m_descriptorsStale = true;
        break;
    default:
        break;
    }
}

void GamepadBackend::openGamepad(quint32 id)
{
    m_descriptorsStale = true;
    if (m_pads.contains(id)) {
        return;
    }
    SDL_Gamepad *pad = SDL_OpenGamepad(id);
    if (!pad) {
        Q_EMIT warning(QStringLiteral("Could not open controller %1: %2").arg(id).arg(sdlError()));
        return;
    }
    m_pads.insert(id, pad);
    Q_EMIT deviceAdded(describeGamepad(id, pad));
}

void GamepadBackend::closeGamepad(quint32 id)
{
    m_descriptorsStale = true;
    SDL_Gamepad *pad = m_pads.take(id);
    if (!pad) {
        return;
    }
    SDL_CloseGamepad(pad);
    m_pendingAxes.removeIf(
        [id](QHash<quint64, PendingAxis>::iterator entry) { return static_cast<quint32>(entry.key() >> AxisKeyShift) == id; });
    Q_EMIT deviceRemoved(id);
}

void GamepadBackend::emitButton(const SDL_Event &event)
{
    const QString name = GamepadButtons::nameForSdlButton(event.gbutton.button);
    if (name.isEmpty()) {
        return;
    }
    flushAxes();
    Q_EMIT buttonChanged(event.gbutton.which, name, event.gbutton.down, nsToMs(event.gbutton.timestamp));
}

void GamepadBackend::queueAxis(const SDL_Event &event)
{
    const quint64 key = (static_cast<quint64>(event.gaxis.which) << AxisKeyShift) | event.gaxis.axis;
    m_pendingAxes.insert(key, {normalizeAxis(event.gaxis.axis, event.gaxis.value), nsToMs(event.gaxis.timestamp)});
}

void GamepadBackend::flushAxes()
{
    for (auto it = m_pendingAxes.cbegin(); it != m_pendingAxes.cend(); ++it) {
        Q_EMIT axisChanged(
            static_cast<quint32>(it.key() >> AxisKeyShift), static_cast<int>(it.key() & AxisKeyMask), it->value, it->timestampMs);
    }
    m_pendingAxes.clear();
}

void GamepadBackend::applyRumbleRequests()
{
    QList<RumbleRequest> requests;
    {
        const QMutexLocker locker(&m_rumbleMutex);
        requests.swap(m_rumbleRequests);
    }
    for (const RumbleRequest &request : std::as_const(requests)) {
        SDL_Gamepad *pad = m_pads.value(request.deviceId);
        if (!pad) {
            continue;
        }
        if (!SDL_RumbleGamepad(pad, request.level, request.level, request.durationMs)) {
            Q_EMIT warning(QStringLiteral("Rumble failed on controller %1: %2").arg(request.deviceId).arg(sdlError()));
            continue;
        }
        const QDeadlineTimer end(static_cast<qint64>(request.durationMs) + RumbleSettleMs);
        if (m_rumbleDeadline.hasExpired() || m_rumbleDeadline.isForever() || end.deadline() > m_rumbleDeadline.deadline()) {
            m_rumbleDeadline = end;
        }
    }
}
