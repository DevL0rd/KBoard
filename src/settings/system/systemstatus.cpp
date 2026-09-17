#include "systemstatus.h"

#include <KConfigGroup>
#include <KSharedConfig>

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusVariant>
#include <QProcess>

namespace
{
const QString KWinService = QStringLiteral("org.kde.KWin");
const QString VirtualKeyboardPath = QStringLiteral("/VirtualKeyboard");
const QString VirtualKeyboardInterface = QStringLiteral("org.kde.kwin.VirtualKeyboard");
const QString PropertiesInterface = QStringLiteral("org.freedesktop.DBus.Properties");
const QString KeyboardService = QStringLiteral("org.devl0rd.KBoard");
}

SystemStatus::SystemStatus(QObject *parent)
    : QObject(parent)
{
    auto bus = QDBusConnection::sessionBus();

    m_keyboardWatcher.setConnection(bus);
    m_keyboardWatcher.addWatchedService(KeyboardService);
    connect(&m_keyboardWatcher, &QDBusServiceWatcher::serviceRegistered, this, [this] {
        m_keyboardRunning = true;
        Q_EMIT keyboardRunningChanged();
    });
    connect(&m_keyboardWatcher, &QDBusServiceWatcher::serviceUnregistered, this, [this] {
        m_keyboardRunning = false;
        Q_EMIT keyboardRunningChanged();
    });
    m_keyboardRunning = bus.interface() && bus.interface()->isServiceRegistered(KeyboardService);

    m_kwinWatcher.setConnection(bus);
    m_kwinWatcher.addWatchedService(KWinService);
    connect(&m_kwinWatcher, &QDBusServiceWatcher::serviceOwnerChanged, this, &SystemStatus::refreshVirtualKeyboard);
    bus.connect(KWinService, VirtualKeyboardPath, PropertiesInterface, QStringLiteral("PropertiesChanged"), this,
        SLOT(onKWinPropertiesChanged(QString, QVariantMap, QStringList)));
    bus.connect(
        KWinService, VirtualKeyboardPath, VirtualKeyboardInterface, QStringLiteral("modeChanged"), this, SLOT(refreshVirtualKeyboard()));
    bus.connect(KWinService, VirtualKeyboardPath, VirtualKeyboardInterface, QStringLiteral("availableChanged"), this,
        SLOT(refreshVirtualKeyboard()));
    refreshVirtualKeyboard();

    m_kwinConfigWatcher = KConfigWatcher::create(KSharedConfig::openConfig(QStringLiteral("kwinrc")));
    connect(m_kwinConfigWatcher.data(), &KConfigWatcher::configChanged, this, [this](const KConfigGroup &group) {
        if (group.name() == QLatin1String("Wayland")) {
            readInputMethod();
        }
    });
    readInputMethod();
}

void SystemStatus::readInputMethod()
{
    const QString value
        = KSharedConfig::openConfig(QStringLiteral("kwinrc"))->group(QStringLiteral("Wayland")).readEntry("InputMethod", QString());
    if (value != m_inputMethod) {
        m_inputMethod = value;
        Q_EMIT inputMethodChanged();
    }
}

void SystemStatus::refreshVirtualKeyboard()
{
    QDBusMessage message = QDBusMessage::createMethodCall(KWinService, VirtualKeyboardPath, PropertiesInterface, QStringLiteral("GetAll"));
    message << VirtualKeyboardInterface;
    auto watcher = new QDBusPendingCallWatcher(QDBusConnection::sessionBus().asyncCall(message), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *call) {
        call->deleteLater();
        QDBusPendingReply<QVariantMap> reply = *call;
        if (reply.isError()) {
            m_kwinReachable = false;
            m_available = false;
            m_mode = -1;
            m_error = QStringLiteral("KWin's virtual keyboard settings can't be reached over D-Bus: %1").arg(reply.error().message());
        } else {
            const QVariantMap values = reply.value();
            m_kwinReachable = true;
            m_available = values.value(QStringLiteral("available")).toBool();
            m_mode = values.value(QStringLiteral("mode"), -1).toInt();
            m_error.clear();
        }
        Q_EMIT virtualKeyboardChanged();
    });
}

void SystemStatus::onKWinPropertiesChanged(const QString &interface, const QVariantMap &, const QStringList &)
{
    if (interface == VirtualKeyboardInterface) {
        refreshVirtualKeyboard();
    }
}

bool SystemStatus::keyboardRunning() const
{
    return m_keyboardRunning;
}

bool SystemStatus::kwinReachable() const
{
    return m_kwinReachable;
}

bool SystemStatus::virtualKeyboardAvailable() const
{
    return m_available;
}

int SystemStatus::virtualKeyboardMode() const
{
    return m_mode;
}

QString SystemStatus::virtualKeyboardError() const
{
    return m_error;
}

QString SystemStatus::inputMethod() const
{
    return m_inputMethod;
}

bool SystemStatus::kboardIsInputMethod() const
{
    return m_inputMethod.contains(QLatin1String("org.devl0rd.kboard"));
}

void SystemStatus::setVirtualKeyboardMode(int mode)
{
    if (mode < Never || mode > AnyInput) {
        m_error = QStringLiteral("Unknown virtual keyboard mode %1.").arg(mode);
        Q_EMIT virtualKeyboardChanged();
        return;
    }
    QDBusMessage message = QDBusMessage::createMethodCall(KWinService, VirtualKeyboardPath, PropertiesInterface, QStringLiteral("Set"));
    message << VirtualKeyboardInterface << QStringLiteral("mode") << QVariant::fromValue(QDBusVariant(mode));
    auto watcher = new QDBusPendingCallWatcher(QDBusConnection::sessionBus().asyncCall(message), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *call) {
        call->deleteLater();
        QDBusPendingReply<> reply = *call;
        if (reply.isError()) {
            m_error = QStringLiteral("KWin refused the change: %1").arg(reply.error().message());
            Q_EMIT virtualKeyboardChanged();
        }
        refreshVirtualKeyboard();
    });
}

void SystemStatus::openVirtualKeyboardSettings()
{
    if (!QProcess::startDetached(QStringLiteral("systemsettings"), {QStringLiteral("kcm_virtualkeyboard")})) {
        m_error = QStringLiteral("Couldn't start System Settings.");
        Q_EMIT virtualKeyboardChanged();
    }
}
