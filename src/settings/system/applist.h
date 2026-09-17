#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QVariantList>

class AppList : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QVariantList applications READ applications NOTIFY applicationsChanged)

public:
    explicit AppList(QObject *parent = nullptr);

    QVariantList applications() const;
    Q_INVOKABLE QVariantMap find(const QString &appId) const;

Q_SIGNALS:
    void applicationsChanged();

private:
    void reload();

    QVariantList m_applications;
};
