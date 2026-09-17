#pragma once

#include <QObject>
#include <QQmlEngine>

class KeyboardService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.devl0rd.KBoard")
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(bool visible READ isVisible WRITE setVisible NOTIFY visibleChanged)
    Q_PROPERTY(QString panel READ panel WRITE setPanel NOTIFY panelChanged)

public:
    static KeyboardService *instance();
    static KeyboardService *create(QQmlEngine *, QJSEngine *);

    bool isVisible() const;
    void setVisible(bool visible);
    QString panel() const;
    void setPanel(const QString &panel);

public Q_SLOTS:
    Q_SCRIPTABLE void Show();
    Q_SCRIPTABLE void Hide();
    Q_SCRIPTABLE void Toggle();
    Q_SCRIPTABLE void OpenPanel(const QString &name);
    Q_SCRIPTABLE bool IsVisible() const;
    Q_SCRIPTABLE QString CurrentPanel() const;
    Q_SCRIPTABLE void OpenSettings(const QString &page);

Q_SIGNALS:
    Q_SCRIPTABLE void VisibleChanged(bool visible);
    Q_SCRIPTABLE void PanelChanged(const QString &panel);
    void visibleChanged();
    void panelChanged();
    void showRequested();
    void hideRequested();

private:
    explicit KeyboardService(QObject *parent = nullptr);
    void forceActivate();

    bool m_visible = false;
    QString m_panel = QStringLiteral("keys");
};
