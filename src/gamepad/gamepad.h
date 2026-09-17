#pragma once

#include "buttonsemantics.h"
#include "devicetracker.h"
#include "gamepaddevice.h"
#include "navigationrepeat.h"

#include <QObject>
#include <QPointF>
#include <QPointer>
#include <QQmlEngine>
#include <QStringList>
#include <QVariantList>

class GamepadBackend;
class KBoardSettings;

class Gamepad : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(bool enabled READ enabled NOTIFY enabledChanged)
    Q_PROPERTY(bool available READ available NOTIFY availableChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorStringChanged)
    Q_PROPERTY(QStringList warnings READ warnings NOTIFY warningsChanged)
    Q_PROPERTY(bool connected READ connected NOTIFY activeDeviceChanged)
    Q_PROPERTY(QString name READ name NOTIFY activeDeviceChanged)
    Q_PROPERTY(QString controllerType READ controllerType NOTIFY activeDeviceChanged)
    Q_PROPERTY(int activeDeviceId READ activeDeviceId NOTIFY activeDeviceChanged)
    Q_PROPERTY(bool rumbleSupported READ rumbleSupported NOTIFY activeDeviceChanged)
    Q_PROPERTY(QVariantList devices READ devices NOTIFY devicesChanged)
    Q_PROPERTY(QStringList heldButtons READ heldButtons NOTIFY heldButtonsChanged)
    Q_PROPERTY(QPointF leftStick READ leftStick NOTIFY leftStickChanged)
    Q_PROPERTY(QPointF rightStick READ rightStick NOTIFY rightStickChanged)
    Q_PROPERTY(double leftStickAngle READ leftStickAngle NOTIFY leftStickChanged)
    Q_PROPERTY(double leftStickMagnitude READ leftStickMagnitude NOTIFY leftStickChanged)
    Q_PROPERTY(double rightStickAngle READ rightStickAngle NOTIFY rightStickChanged)
    Q_PROPERTY(double rightStickMagnitude READ rightStickMagnitude NOTIFY rightStickChanged)
    Q_PROPERTY(double leftTrigger READ leftTrigger NOTIFY leftTriggerChanged)
    Q_PROPERTY(double rightTrigger READ rightTrigger NOTIFY rightTriggerChanged)
    Q_PROPERTY(double deadzone READ deadzone NOTIFY deadzoneChanged)
    Q_PROPERTY(bool radialMode READ radialMode NOTIFY radialModeChanged)
    Q_PROPERTY(QStringList chord READ chord NOTIFY chordChanged)
    Q_PROPERTY(QString chordError READ chordError NOTIFY chordChanged)
    Q_PROPERTY(QStringList buttonNames READ buttonNames CONSTANT)
    Q_PROPERTY(QStringList controllerTypes READ controllerTypes CONSTANT)

public:
    enum class Mode
    {
        WithBackend,
        Detached,
    };

    explicit Gamepad(Mode mode, QObject *parent = nullptr);
    ~Gamepad() override;

    static Gamepad *instance();
    static Gamepad *create(QQmlEngine *engine, QJSEngine *scriptEngine);

    bool enabled() const;
    bool available() const { return m_available; }
    QString errorString() const { return m_errorString; }
    QStringList warnings() const { return m_warnings; }
    bool connected() const { return m_devices.active() != nullptr; }
    QString name() const;
    QString controllerType() const;
    int activeDeviceId() const { return static_cast<int>(m_devices.activeId()); }
    bool rumbleSupported() const;
    QVariantList devices() const { return m_devices.toVariantList(); }
    QStringList heldButtons() const;
    QPointF leftStick() const;
    QPointF rightStick() const;
    double leftStickAngle() const { return stickAngle(leftStick()); }
    double leftStickMagnitude() const { return stickMagnitude(leftStick()); }
    double rightStickAngle() const { return stickAngle(rightStick()); }
    double rightStickMagnitude() const { return stickMagnitude(rightStick()); }
    double leftTrigger() const;
    double rightTrigger() const;
    double deadzone() const;
    bool radialMode() const;
    QStringList chord() const { return m_buttons.chord(); }
    QString chordError() const { return m_buttons.chordError(); }
    QStringList buttonNames() const;
    QStringList controllerTypes() const;

    Q_INVOKABLE QString glyph(const QString &button) const;
    Q_INVOKABLE QString glyphFor(const QString &controllerType, const QString &button) const;
    Q_INVOKABLE bool isHeld(const QString &button) const;
    Q_INVOKABLE bool rumble(double strength, int durationMs);

    static double stickAngle(QPointF stick);
    static double stickMagnitude(QPointF stick);

public Q_SLOTS:
    void handleDeviceAdded(const GamepadDevice &device);
    void handleDeviceRemoved(quint32 id);
    void handleButton(quint32 id, const QString &button, bool down, qint64 timestampMs);
    void handleAxis(quint32 id, int axis, double value, qint64 timestampMs);

Q_SIGNALS:
    void enabledChanged();
    void availableChanged();
    void errorStringChanged();
    void warningsChanged();
    void activeDeviceChanged();
    void devicesChanged();
    void heldButtonsChanged();
    void leftStickChanged();
    void rightStickChanged();
    void leftTriggerChanged();
    void rightTriggerChanged();
    void deadzoneChanged();
    void radialModeChanged();
    void chordChanged();
    void buttonPressed(const QString &button);
    void buttonReleased(const QString &button);
    void buttonTapped(const QString &button);
    void navigate(int dx, int dy);
    void chordActivated();

private:
    void connectSettings();
    void applyEnabled();
    void startBackend();
    void stopBackend();
    void setAvailable(bool available);
    void setErrorString(const QString &error);
    void addWarning(const QString &warning);
    void applyChord();
    void activate(quint32 id);
    void releaseActiveState();
    void pressButton(quint32 id, const QString &button, qint64 timestampMs);
    void releaseButton(quint32 id, const QString &button);
    void updateTriggerButton(quint32 id, int axis, double value, qint64 timestampMs);
    void updateDpad();
    void updateLeftStickNavigation();
    void emitAxisChanged(int axis);
    void emitAllAxesChanged();
    void clearDevices();
    double axisValue(int axis) const;
    QPointF stickValue(int xAxis, int yAxis) const;
    bool isActivity(const DeviceState &state, int axis) const;

    KBoardSettings *m_settings;
    Mode m_mode;
    QPointer<GamepadBackend> m_backend;
    QPointer<QObject> m_backendContext;
    bool m_available = false;
    QString m_errorString;
    QStringList m_warnings;
    DeviceTracker m_devices;
    ButtonSemantics m_buttons;
    NavigationRepeater m_repeater;
};
