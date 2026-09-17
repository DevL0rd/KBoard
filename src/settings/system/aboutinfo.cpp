#include "aboutinfo.h"

#include <kboard_version.h>

#include <KAboutData>
#include <KLocalizedString>

#include <QIcon>

namespace AboutInfo
{

KAboutData create()
{
    KAboutData about(QStringLiteral("kboard-settings"), i18n("KBoard Settings"), QStringLiteral(KBOARD_VERSION_STRING),
        i18n("Configure the KBoard on-screen keyboard"), KAboutLicense::GPL_V3, i18n("© 2026 DevL0rd"));
    about.addAuthor(
        QStringLiteral("DevL0rd"), i18n("Author"), QStringLiteral("dmhzmxn@gmail.com"), QStringLiteral("https://github.com/DevL0rd"));
    about.setHomepage(QStringLiteral("https://github.com/DevL0rd/KBoard"));
    about.setBugAddress(QByteArrayLiteral("https://github.com/DevL0rd/KBoard/issues"));
    about.setDesktopFileName(QStringLiteral("org.devl0rd.kboard.settings"));
    about.setProgramLogo(QIcon::fromTheme(QStringLiteral("input-keyboard-virtual")));
    return about;
}

void install()
{
    KLocalizedString::setApplicationDomain("kboard");
    KAboutData::setApplicationData(create());
}

}
