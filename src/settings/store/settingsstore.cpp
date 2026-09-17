#include "settingsstore.h"

#include "kboardsettings.h"

#include <KConfig>
#include <KConfigGroup>
#include <KCoreConfigSkeleton>

#include <QColor>
#include <QCoreApplication>
#include <QFileInfo>
#include <QMetaMethod>
#include <QMetaProperty>
#include <QStandardPaths>
#include <algorithm>
#include <ranges>

namespace
{
const QString ExportGroup = QStringLiteral("KBoard Settings Export");
}

SettingsStore::SettingsStore(QObject *parent)
    : QObject(parent)
    , m_skeleton(KBoardSettings::self())
{
    m_clock.start();
    m_saveTimer.setSingleShot(true);
    m_saveTimer.setInterval(120);
    connect(&m_saveTimer, &QTimer::timeout, this, &SettingsStore::flush);
    connect(qApp, &QCoreApplication::aboutToQuit, this, &SettingsStore::flush);
    m_watcher = KConfigWatcher::create(m_skeleton->sharedConfig());
    connect(m_watcher.data(), &KConfigWatcher::configChanged, this, &SettingsStore::reloadFromDisk);
    const QMetaObject *meta = m_skeleton->metaObject();
    const QMetaMethod bumpMethod = metaObject()->method(metaObject()->indexOfMethod("bump()"));
    for (int i = meta->propertyOffset(); i < meta->propertyCount(); ++i) {
        const QMetaProperty property = meta->property(i);
        if (property.hasNotifySignal()) {
            connect(m_skeleton, property.notifySignal(), this, bumpMethod);
        }
    }
    connect(m_skeleton, &KCoreConfigSkeleton::configChanged, this, &SettingsStore::bump);
}

SettingsStore *SettingsStore::instance()
{
    static SettingsStore *store = new SettingsStore;
    return store;
}

SettingsStore *SettingsStore::create(QQmlEngine *, QJSEngine *)
{
    QJSEngine::setObjectOwnership(instance(), QJSEngine::CppOwnership);
    return instance();
}

void SettingsStore::reloadFromDisk()
{
    if (m_saveTimer.isActive()) {
        return;
    }
    m_skeleton->sharedConfig()->reparseConfiguration();
    m_skeleton->load();
}

void SettingsStore::bump()
{
    ++m_revision;
    Q_EMIT revisionChanged();
}

int SettingsStore::revision() const
{
    return m_revision;
}

bool SettingsStore::canUndo() const
{
    return m_history.canUndo();
}

int SettingsStore::undoCount() const
{
    return int(m_history.size());
}

bool SettingsStore::representsDefaults() const
{
    const auto items = m_skeleton->items();
    return std::all_of(items.cbegin(), items.cend(), [](KConfigSkeletonItem *entry) { return entry->isDefault(); });
}

QString SettingsStore::configPath() const
{
    return QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QLatin1Char('/') + m_skeleton->sharedConfig()->name();
}

QStringList SettingsStore::entryNames() const
{
    QStringList names;
    const auto items = m_skeleton->items();
    for (KConfigSkeletonItem *entry : items) {
        names.append(entry->name());
    }
    return names;
}

KConfigSkeletonItem *SettingsStore::item(const QString &name) const
{
    return m_skeleton->findItem(name);
}

bool SettingsStore::has(const QString &name) const
{
    return item(name) != nullptr;
}

QString SettingsStore::groupOf(const QString &name) const
{
    auto entry = item(name);
    return entry ? entry->group() : QString();
}

QVariant SettingsStore::value(const QString &name) const
{
    auto entry = item(name);
    return entry ? entry->property() : QVariant();
}

QVariant SettingsStore::normalized(KConfigSkeletonItem *entry, const QVariant &value) const
{
    const QVariant current = entry->property();
    QVariant result = value;
    if (current.metaType() != result.metaType()) {
        if (current.metaType().id() == QMetaType::QStringList && result.canConvert<QStringList>()) {
            result = result.toStringList();
        } else if (current.metaType().id() == QMetaType::QColor) {
            result = QColor(value.toString());
        } else if (!result.convert(current.metaType())) {
            return {};
        }
    }
    const QVariant low = entry->minValue();
    const QVariant high = entry->maxValue();
    if (low.isValid() && QVariant::compare(result, low) == QPartialOrdering::Less) {
        result = low;
    }
    if (high.isValid() && QVariant::compare(result, high) == QPartialOrdering::Greater) {
        result = high;
    }
    return result;
}

bool SettingsStore::setValue(const QString &name, const QVariant &value)
{
    auto entry = item(name);
    if (!entry) {
        Q_EMIT failed(QStringLiteral("There is no setting called \"%1\".").arg(name));
        return false;
    }
    const QVariant next = normalized(entry, value);
    if (!next.isValid()) {
        Q_EMIT failed(QStringLiteral("\"%1\" can't be set to that value.").arg(name));
        return false;
    }
    if (entry->isEqual(next)) {
        return true;
    }
    m_history.record(name, entry->property(), m_clock.elapsed());
    entry->setProperty(next);
    commit(QStringLiteral("Applied"));
    return true;
}

bool SettingsStore::setValues(const QVariantMap &values)
{
    QList<std::pair<KConfigSkeletonItem *, QVariant>> updates;
    for (auto it = values.cbegin(); it != values.cend(); ++it) {
        auto entry = item(it.key());
        const QVariant next = entry ? normalized(entry, it.value()) : QVariant();
        if (!next.isValid()) {
            Q_EMIT failed(QStringLiteral("\"%1\" can't be set to that value.").arg(it.key()));
            return false;
        }
        updates.append({entry, next});
    }
    QList<EditChange> changes;
    for (const auto &[entry, next] : std::as_const(updates)) {
        if (!entry->isEqual(next)) {
            changes.append({entry->name(), entry->property()});
            entry->setProperty(next);
        }
    }
    if (changes.isEmpty()) {
        return true;
    }
    m_history.recordStep(changes);
    commit(QStringLiteral("Applied"));
    return true;
}

bool SettingsStore::isDefault(const QString &name) const
{
    auto entry = item(name);
    return entry && entry->isDefault();
}

QVariant SettingsStore::defaultValue(const QString &name) const
{
    auto entry = item(name);
    if (!entry) {
        return {};
    }
    return entry->getDefault();
}

bool SettingsStore::resetToDefault(const QString &name)
{
    auto entry = item(name);
    if (!entry) {
        Q_EMIT failed(QStringLiteral("There is no setting called \"%1\".").arg(name));
        return false;
    }
    if (entry->isDefault()) {
        return true;
    }
    m_history.recordStep({EditChange {name, entry->property()}});
    entry->setDefault();
    commit(QStringLiteral("Reset to default"));
    return true;
}

void SettingsStore::defaults()
{
    QList<EditChange> changes;
    const auto items = m_skeleton->items();
    for (KConfigSkeletonItem *entry : items) {
        if (!entry->isDefault()) {
            changes.append({entry->name(), entry->property()});
            entry->setDefault();
        }
    }
    if (changes.isEmpty()) {
        return;
    }
    m_history.recordStep(changes);
    commit(QStringLiteral("Everything is back to defaults"));
}

bool SettingsStore::undo()
{
    auto step = m_history.undo();
    if (!step) {
        Q_EMIT historyChanged();
        return false;
    }
    for (const EditChange &change : std::views::reverse(*step)) {
        if (auto entry = item(change.name)) {
            entry->setProperty(change.before);
        }
    }
    commit(QStringLiteral("Undone"));
    return true;
}

void SettingsStore::commit(const QString &message)
{
    m_pendingMessage = message;
    m_saveTimer.start();
    Q_EMIT historyChanged();
    bump();
}

void SettingsStore::flush()
{
    if (m_pendingMessage.isEmpty()) {
        return;
    }
    m_saveTimer.stop();
    const QString message = std::exchange(m_pendingMessage, QString());
    if (!m_skeleton->save()) {
        Q_EMIT failed(QStringLiteral("Couldn't write %1. Check that the file is writable.").arg(configPath()));
        return;
    }
    Q_EMIT applied(message);
}

bool SettingsStore::exportToFile(const QString &path, QString *error) const
{
    KConfig target(path, KConfig::SimpleConfig);
    const QStringList groups = target.groupList();
    for (const QString &group : groups) {
        target.deleteGroup(group);
    }
    KConfigGroup marker(&target, ExportGroup);
    marker.writeEntry("Version", 1);
    const auto items = m_skeleton->items();
    for (KConfigSkeletonItem *entry : items) {
        KConfigGroup group(&target, entry->group());
        group.writeEntry(entry->key().toUtf8().constData(), entry->property());
    }
    if (!target.sync()) {
        if (error) {
            *error = QStringLiteral("Couldn't write %1.").arg(path);
        }
        return false;
    }
    return true;
}

bool SettingsStore::importFromFile(const QString &path, QString *error)
{
    if (!QFileInfo(path).isReadable()) {
        if (error) {
            *error = QStringLiteral("Couldn't read %1.").arg(path);
        }
        return false;
    }
    KConfig source(path, KConfig::SimpleConfig);
    const QStringList groups = source.groupList();
    const auto items = m_skeleton->items();
    const bool known = groups.contains(ExportGroup)
        || std::any_of(items.cbegin(), items.cend(), [&groups](KConfigSkeletonItem *entry) { return groups.contains(entry->group()); });
    if (!known) {
        if (error) {
            *error = QStringLiteral("%1 doesn't contain KBoard settings.").arg(QFileInfo(path).fileName());
        }
        return false;
    }
    QList<EditChange> changes;
    for (KConfigSkeletonItem *entry : items) {
        const QVariant before = entry->property();
        entry->readConfig(&source);
        if (!entry->isEqual(before)) {
            changes.append({entry->name(), before});
        }
    }
    m_history.recordStep(changes);
    return true;
}

bool SettingsStore::exportTo(const QUrl &url)
{
    QString error;
    if (!exportToFile(url.toLocalFile(), &error)) {
        Q_EMIT failed(error);
        return false;
    }
    Q_EMIT applied(QStringLiteral("Exported to %1").arg(url.fileName()));
    return true;
}

bool SettingsStore::importFrom(const QUrl &url)
{
    QString error;
    if (!importFromFile(url.toLocalFile(), &error)) {
        Q_EMIT failed(error);
        return false;
    }
    commit(QStringLiteral("Imported %1").arg(url.fileName()));
    return true;
}
