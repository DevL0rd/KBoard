#pragma once

#include "descriptorscanner.h"
#include "gamepaddevice.h"
#include "hotplugmonitor.h"
#include "pollset.h"

#include <QAtomicInteger>
#include <QDeadlineTimer>
#include <QHash>
#include <QList>
#include <QMutex>
#include <QStringList>
#include <QThread>

struct SDL_Gamepad;
union SDL_Event;

class GamepadBackend : public QThread
{
    Q_OBJECT

public:
    explicit GamepadBackend(const QStringList &mappingFiles, QObject *parent = nullptr);
    ~GamepadBackend() override;

    void requestStop();
    void rumble(quint32 deviceId, double strength, int durationMs);

    static int nextTimeoutMs(const QDeadlineTimer &deadline);

Q_SIGNALS:
    void initialized();
    void failed(const QString &error);
    void warning(const QString &message);
    void deviceAdded(const GamepadDevice &device);
    void deviceRemoved(quint32 id);
    void buttonChanged(quint32 id, const QString &button, bool down, qint64 timestampMs);
    void axisChanged(quint32 id, int axis, double value, qint64 timestampMs);

protected:
    void run() override;

private:
    struct RumbleRequest
    {
        quint32 deviceId = 0;
        quint16 level = 0;
        quint32 durationMs = 0;
    };

    struct PendingAxis
    {
        double value = 0.0;
        qint64 timestampMs = 0;
    };

    bool initialize();
    void loadMappings();
    void eventLoop();
    void shutdown();
    void refreshDescriptors();
    void dispatchQueued();
    void processEvent(const SDL_Event &event);
    void openGamepad(quint32 id);
    void closeGamepad(quint32 id);
    void emitButton(const SDL_Event &event);
    void queueAxis(const SDL_Event &event);
    void flushAxes();
    void applyRumbleRequests();

    QStringList m_mappingFiles;
    HotplugMonitor m_hotplug;
    PollSet m_pollSet;
    DescriptorScanner::Snapshot m_baseline;
    QAtomicInteger<bool> m_stopRequested = false;
    QMutex m_rumbleMutex;
    QList<RumbleRequest> m_rumbleRequests;
    QHash<quint32, SDL_Gamepad *> m_pads;
    QHash<quint64, PendingAxis> m_pendingAxes;
    QDeadlineTimer m_rumbleDeadline;
    bool m_descriptorsStale = true;
};
