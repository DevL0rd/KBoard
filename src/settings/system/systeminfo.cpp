#include "systeminfo.h"

#include <KAboutData>
#include <KCoreAddons>

QString SystemInfo::qtVersion()
{
    return QString::fromLatin1(qVersion());
}

QString SystemInfo::frameworksVersion()
{
    return KCoreAddons::versionString();
}

QString SystemInfo::displayName()
{
    return KAboutData::applicationData().displayName();
}

QString SystemInfo::version()
{
    return KAboutData::applicationData().version();
}

QString SystemInfo::shortDescription()
{
    return KAboutData::applicationData().shortDescription();
}

QString SystemInfo::bugAddress()
{
    return KAboutData::applicationData().bugAddress();
}

QVariantList SystemInfo::authors()
{
    QVariantList list;
    const auto people = KAboutData::applicationData().authors();
    for (const KAboutPerson &person : people) {
        list.append(QVariantMap {
            {QStringLiteral("name"), person.name()},
            {QStringLiteral("task"), person.task()},
            {QStringLiteral("emailAddress"), person.emailAddress()},
            {QStringLiteral("webAddress"), person.webAddress()},
        });
    }
    return list;
}
