#include "activeapp.h"

#include "kboardpaths.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QFile>

namespace
{
const QString s_service = QStringLiteral("org.kde.KWin");
const QString s_scripting = QStringLiteral("/Scripting");
}

ActiveAppWatcher::ActiveAppWatcher(QObject *parent)
    : QObject(parent)
    , m_pluginName(QStringLiteral("kboard-active-app"))
{ }

void ActiveAppWatcher::start()
{
    const QString script = KBoardPaths::dataFile(QStringLiteral("kwin/activeapp.js"));
    if (!QFile::exists(script)) {
        qWarning() << "KBoard: cannot watch the active application, missing" << script;
        return;
    }
    QDBusMessage message
        = QDBusMessage::createMethodCall(s_service, s_scripting, QStringLiteral("org.kde.kwin.Scripting"), QStringLiteral("loadScript"));
    message << script << m_pluginName;
    auto *watcher = new QDBusPendingCallWatcher(QDBusConnection::sessionBus().asyncCall(message), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *call) {
        const QDBusPendingReply<int> reply = *call;
        call->deleteLater();
        if (reply.isError()) {
            qWarning() << "KBoard: KWin rejected the active application script:" << reply.error().message();
            return;
        }
        runScript(reply.value());
    });
}

void ActiveAppWatcher::runScript(int id)
{
    const QString path = QStringLiteral("/Scripting/Script%1").arg(id);
    QDBusConnection::sessionBus().asyncCall(
        QDBusMessage::createMethodCall(s_service, path, QStringLiteral("org.kde.kwin.Script"), QStringLiteral("run")));
}
