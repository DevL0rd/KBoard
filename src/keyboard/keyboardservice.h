#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QTimer>

class KeyboardService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.devl0rd.KBoard")
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(bool visible READ isVisible WRITE setVisible NOTIFY visibleChanged)
    Q_PROPERTY(QString panel READ panel WRITE setPanel NOTIFY panelChanged)
    Q_PROPERTY(bool gestureActive READ gestureActive NOTIFY gestureActiveChanged)
    Q_PROPERTY(QString keyMap MEMBER m_keyMap)
    Q_PROPERTY(QString activeApp READ activeApp NOTIFY activeAppChanged)

public:
    static KeyboardService *instance();
    static KeyboardService *create(QQmlEngine *, QJSEngine *);

    bool isVisible() const;
    void setVisible(bool visible);
    QString panel() const;
    bool gestureActive() const;
    QString activeApp() const;
    void setPanel(const QString &panel);

public Q_SLOTS:
    Q_SCRIPTABLE void Show();
    Q_SCRIPTABLE void Hide();
    Q_SCRIPTABLE void Toggle();
    Q_SCRIPTABLE void OpenPanel(const QString &name);
    Q_SCRIPTABLE bool IsVisible() const;
    Q_SCRIPTABLE QString CurrentPanel() const;
    Q_SCRIPTABLE void OpenSettings(const QString &page);
    Q_SCRIPTABLE void SetEnabled(bool enabled);
    Q_SCRIPTABLE void Enable();
    Q_SCRIPTABLE void Disable();
    Q_SCRIPTABLE bool IsEnabled() const;
    Q_SCRIPTABLE void Deactivate();
    Q_SCRIPTABLE QString KeyMap();
    Q_SCRIPTABLE void SetActiveApp(const QString &application);

Q_SIGNALS:
    Q_SCRIPTABLE void VisibleChanged(bool visible);
    Q_SCRIPTABLE void PanelChanged(const QString &panel);
    Q_SCRIPTABLE void EnabledChanged(bool enabled);
    void visibleChanged();
    void panelChanged();
    void gestureActiveChanged();
    void activeAppChanged();
    void showRequested();
    void hideRequested();
    void keyMapRequested();

private Q_SLOTS:
    void handleMultiTouch(bool active);

private:
    explicit KeyboardService(QObject *parent = nullptr);
    void watchGestures();
    void forceActivate();
    void deactivate();

    bool m_visible = false;
    QString m_panel = QStringLiteral("keys");
    bool m_gestureActive = false;
    QTimer m_gestureRelease;
    QString m_keyMap;
    QString m_activeApp;
};
