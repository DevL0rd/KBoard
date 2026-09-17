#include "keyboardservice.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QProcess>

KeyboardService::KeyboardService(QObject *parent)
    : QObject(parent)
{
}

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

void KeyboardService::forceActivate()
{
    auto message = QDBusMessage::createMethodCall(QStringLiteral("org.kde.KWin"), QStringLiteral("/VirtualKeyboard"), QStringLiteral("org.kde.kwin.VirtualKeyboard"), QStringLiteral("forceActivate"));
    QDBusConnection::sessionBus().asyncCall(message);
}

void KeyboardService::Show()
{
    forceActivate();
    Q_EMIT showRequested();
}

void KeyboardService::Hide()
{
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
    QProcess::startDetached(QStringLiteral("kboard-settings"), arguments);
}
