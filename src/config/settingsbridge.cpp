#include "settingsbridge.h"

#include <KConfigWatcher>

namespace
{
KConfigSkeletonItem *itemNamed(const QString &name)
{
    return KBoardSettings::self()->findItem(name);
}
}

SettingsWatcher::SettingsWatcher(QObject *parent)
    : QObject(parent)
{
    startWatching();
}

void SettingsWatcher::startWatching()
{
    static KConfigWatcher::Ptr watcher;
    if (watcher) {
        return;
    }
    watcher = KConfigWatcher::create(KBoardSettings::self()->sharedConfig());
    QObject::connect(watcher.data(), &KConfigWatcher::configChanged, KBoardSettings::self(), [] {
        KBoardSettings::self()->sharedConfig()->reparseConfiguration();
        KBoardSettings::self()->load();
    });
}

void SettingsWatcher::save()
{
    KBoardSettings::self()->save();
}

void SettingsWatcher::resetToDefault(const QString &name)
{
    if (auto item = itemNamed(name)) {
        item->setDefault();
        KBoardSettings::self()->save();
    }
}

bool SettingsWatcher::isDefault(const QString &name) const
{
    auto item = itemNamed(name);
    return !item || item->isDefault();
}

QVariant SettingsWatcher::defaultValue(const QString &name) const
{
    auto item = itemNamed(name);
    if (!item) {
        return {};
    }
    item->swapDefault();
    QVariant value = item->property();
    item->swapDefault();
    return value;
}

void SettingsWatcher::setValue(const QString &name, const QVariant &value)
{
    if (auto item = itemNamed(name)) {
        item->setProperty(value);
        KBoardSettings::self()->save();
    }
}
