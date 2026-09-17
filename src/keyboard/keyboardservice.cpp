#include "keyboardservice.h"

#include "kboardsettings.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QDBusVariant>
#include <QProcess>

KeyboardService::KeyboardService(QObject *parent)
    : QObject(parent)
{ }

KeyboardService *KeyboardService::instance()
{
    static KeyboardService *service = new KeyboardService;
    return service;
}

KeyboardService *KeyboardService::create(QQmlEngine *, QJSEngine *)
{
    QJSEngine::setObjectOwnership(instance(), QJSEngine::CppOwnership);
    return instance();
}

bool KeyboardService::isVisible() const
{
    return m_visible;
}

void KeyboardService::setVisible(bool visible)
{
    if (visible == m_visible) {
        return;
    }
    m_visible = visible;
    Q_EMIT visibleChanged();
    Q_EMIT VisibleChanged(visible);
}

QString KeyboardService::panel() const
{
    return m_panel;
}

void KeyboardService::setPanel(const QString &panel)
{
    if (panel == m_panel) {
        return;
    }
    m_panel = panel;
    Q_EMIT panelChanged();
    Q_EMIT PanelChanged(panel);
}

namespace
{
QDBusMessage virtualKeyboardCall(const QString &interface, const QString &method)
{
    return QDBusMessage::createMethodCall(QStringLiteral("org.kde.KWin"), QStringLiteral("/VirtualKeyboard"), interface, method);
}
}

void KeyboardService::forceActivate()
{
    QDBusConnection::sessionBus().asyncCall(
        virtualKeyboardCall(QStringLiteral("org.kde.kwin.VirtualKeyboard"), QStringLiteral("forceActivate")));
}

void KeyboardService::deactivate()
{
    QDBusMessage message = virtualKeyboardCall(QStringLiteral("org.freedesktop.DBus.Properties"), QStringLiteral("Set"));
    message << QStringLiteral("org.kde.kwin.VirtualKeyboard") << QStringLiteral("active") << QVariant::fromValue(QDBusVariant(false));
    QDBusConnection::sessionBus().asyncCall(message);
}

void KeyboardService::Deactivate()
{
    deactivate();
}

void KeyboardService::SetEnabled(bool enabled)
{
    if (enabled == KBoardSettings::enabled()) {
        return;
    }
    KBoardSettings::setEnabled(enabled);
    KBoardSettings::self()->save();
    Q_EMIT EnabledChanged(enabled);
    if (enabled) {
        Show();
    } else {
        Hide();
    }
}

bool KeyboardService::IsEnabled() const
{
    return KBoardSettings::enabled();
}

void KeyboardService::Show()
{
    forceActivate();
    Q_EMIT showRequested();
}

void KeyboardService::Hide()
{
    deactivate();
    Q_EMIT hideRequested();
}

void KeyboardService::Toggle()
{
    if (m_visible) {
        Hide();
    } else {
        Show();
    }
}

void KeyboardService::OpenPanel(const QString &name)
{
    setPanel(name);
    if (!m_visible) {
        Show();
    }
}

bool KeyboardService::IsVisible() const
{
    return m_visible;
}

QString KeyboardService::CurrentPanel() const
{
    return m_panel;
}

void KeyboardService::OpenSettings(const QString &page)
{
    QStringList arguments;
    if (!page.isEmpty()) {
        arguments << QStringLiteral("--page") << page;
    }
    QProcess::startDetached(QStringLiteral(KBOARD_SETTINGS_BINARY), arguments);
}

QString KeyboardService::KeyMap()
{
    Q_EMIT keyMapRequested();
    return m_keyMap;
}

QString KeyboardService::activeApp() const
{
    return m_activeApp;
}

void KeyboardService::SetActiveApp(const QString &application)
{
    if (application == m_activeApp) {
        return;
    }
    m_activeApp = application;
    Q_EMIT activeAppChanged();
}
