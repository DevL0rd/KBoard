#include "applist.h"

#include <KApplicationTrader>
#include <KSycoca>

#include <algorithm>

AppList::AppList(QObject *parent)
    : QObject(parent)
{
    connect(KSycoca::self(), &KSycoca::databaseChanged, this, &AppList::reload);
    reload();
}

void AppList::reload()
{
    const KService::List services
        = KApplicationTrader::query([](const KService::Ptr &service) { return !service->noDisplay() && !service->name().isEmpty(); });
    QList<QVariantMap> apps;
    for (const KService::Ptr &service : services) {
        QString id = service->desktopEntryName();
        apps.append({
            {QStringLiteral("appId"), id},
            {QStringLiteral("name"), service->name()},
            {QStringLiteral("icon"), service->icon()},
            {QStringLiteral("comment"), service->genericName().isEmpty() ? service->comment() : service->genericName()},
        });
    }
    std::sort(apps.begin(), apps.end(), [](const QVariantMap &a, const QVariantMap &b) {
        return a.value(QStringLiteral("name")).toString().localeAwareCompare(b.value(QStringLiteral("name")).toString()) < 0;
    });
    m_applications.clear();
    for (const QVariantMap &app : std::as_const(apps)) {
        m_applications.append(app);
    }
    Q_EMIT applicationsChanged();
}

QVariantList AppList::applications() const
{
    return m_applications;
}

QVariantMap AppList::find(const QString &appId) const
{
    for (const QVariant &app : m_applications) {
        const QVariantMap map = app.toMap();
        if (map.value(QStringLiteral("appId")).toString() == appId) {
            return map;
        }
    }
    return {};
}
