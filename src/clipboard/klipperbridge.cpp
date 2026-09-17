#include "klipperbridge.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusServiceWatcher>

namespace
{
const QString s_service = QStringLiteral("org.kde.klipper");
const QString s_path = QStringLiteral("/klipper");
const QString s_interface = QStringLiteral("org.kde.klipper.klipper");
}

KlipperBridge::KlipperBridge(QObject *parent)
    : QObject(parent)
{
    auto bus = QDBusConnection::sessionBus();
    m_watcher = new QDBusServiceWatcher(s_service, bus, QDBusServiceWatcher::WatchForOwnerChange, this);
    connect(m_watcher, &QDBusServiceWatcher::serviceRegistered, this, [this] {
        setConnected(true);
        fetch(false);
    });
    connect(m_watcher, &QDBusServiceWatcher::serviceUnregistered, this, [this] { setConnected(false); });
    bus.connect(s_service, s_path, s_interface, QStringLiteral("clipboardHistoryUpdated"), this, SLOT(onHistoryUpdated()));
    if (bus.interface() && bus.interface()->isServiceRegistered(s_service)) {
        setConnected(true);
    }
}

bool KlipperBridge::isConnected() const
{
    return m_connected;
}

void KlipperBridge::onHistoryUpdated()
{
    fetch(true);
}

void KlipperBridge::fetch(bool onlyNewest)
{
    if (!m_connected) {
        return;
    }
    const QString method = onlyNewest ? QStringLiteral("getClipboardContents") : QStringLiteral("getClipboardHistoryMenu");
    const auto message = QDBusMessage::createMethodCall(s_service, s_path, s_interface, method);
    auto watcher = new QDBusPendingCallWatcher(QDBusConnection::sessionBus().asyncCall(message), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, watcher, onlyNewest, method] {
        watcher->deleteLater();
        const QDBusMessage reply = watcher->reply();
        if (reply.type() == QDBusMessage::ErrorMessage || reply.arguments().isEmpty()) {
            qWarning() << "kboard clipboard: Klipper" << method << "failed" << reply.errorMessage();
            return;
        }
        const QVariant value = reply.arguments().constFirst();
        Q_EMIT historyReceived(onlyNewest ? QStringList {value.toString()} : value.toStringList(), onlyNewest);
    });
}

void KlipperBridge::setConnected(bool connected)
{
    if (m_connected == connected) {
        return;
    }
    m_connected = connected;
    Q_EMIT connectedChanged();
}
