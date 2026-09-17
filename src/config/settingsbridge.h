#pragma once

#include "kboardsettings.h"

#include <QObject>
#include <QQmlEngine>

class SettingsForeign
{
    Q_GADGET
    QML_FOREIGN(KBoardSettings)
    QML_NAMED_ELEMENT(Settings)
    QML_SINGLETON

public:
    static KBoardSettings *create(QQmlEngine *, QJSEngine *)
    {
        QJSEngine::setObjectOwnership(KBoardSettings::self(), QJSEngine::CppOwnership);
        return KBoardSettings::self();
    }
};

class SettingsWatcher : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    explicit SettingsWatcher(QObject *parent = nullptr);
    static void startWatching();
    Q_INVOKABLE void save();
    Q_INVOKABLE void resetToDefault(const QString &name);
    Q_INVOKABLE bool isDefault(const QString &name) const;
    Q_INVOKABLE QVariant defaultValue(const QString &name) const;
    Q_INVOKABLE void setValue(const QString &name, const QVariant &value);

Q_SIGNALS:
    void reloaded();
};
