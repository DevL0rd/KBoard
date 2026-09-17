#include "haptics.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusServiceWatcher>
#include <QVariantMap>

namespace
{
const QString feedbackService = QStringLiteral("org.sigxcpu.Feedback");
const QString feedbackPath = QStringLiteral("/org/sigxcpu/Feedback");
const QString appId = QStringLiteral("org.devl0rd.kboard");
}

Haptics::Haptics(QObject *parent)
    : QObject(parent)
    , m_watcher(new QDBusServiceWatcher(feedbackService, QDBusConnection::sessionBus(),
          QDBusServiceWatcher::WatchForRegistration | QDBusServiceWatcher::WatchForUnregistration, this))
{
    connect(m_watcher, &QDBusServiceWatcher::serviceRegistered, this, [this] { setAvailable(true); });
    connect(m_watcher, &QDBusServiceWatcher::serviceUnregistered, this, [this] { setAvailable(false); });

    if (!QDBusConnection::sessionBus().isConnected()) {
        return;
    }
    const QDBusMessage query
        = QDBusMessage::createMethodCall(QStringLiteral("org.freedesktop.DBus"), QStringLiteral("/org/freedesktop/DBus"),
              QStringLiteral("org.freedesktop.DBus"), QStringLiteral("NameHasOwner"))
        << feedbackService;
    auto watcher = new QDBusPendingCallWatcher(QDBusConnection::sessionBus().asyncCall(query), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *call) {
        const QDBusPendingReply<bool> reply = *call;
        if (!reply.isError() && reply.value()) {
            setAvailable(true);
        }
        call->deleteLater();
    });
}

bool Haptics::available() const
{
    return m_available;
}

void Haptics::trigger(const QString &event)
{
    if (!m_available) {
        return;
    }
    const QDBusMessage message
        = QDBusMessage::createMethodCall(feedbackService, feedbackPath, feedbackService, QStringLiteral("TriggerFeedback"))
        << appId << event << QVariantMap() << -1;
    QDBusConnection::sessionBus().asyncCall(message);
}

void Haptics::setAvailable(bool available)
{
    if (m_available == available) {
        return;
    }
    m_available = available;
    Q_EMIT availableChanged();
}
