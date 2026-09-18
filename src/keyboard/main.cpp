#include "activeapp.h"
#include "keyboardservice.h"
#include "qmlengine.h"
#include "settingsbridge.h"

#include <kboard_version.h>

#include <KAboutData>
#include <KCrash>
#include <KLocalizedString>

#include <QDBusConnection>
#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQuickStyle>

int main(int argc, char **argv)
{
    qunsetenv("QT_IM_MODULE");
    QGuiApplication application(argc, argv);
    KLocalizedString::setApplicationDomain("kboard");

    KAboutData about(QStringLiteral("kboard"), i18n("KBoard"), QStringLiteral(KBOARD_VERSION_STRING), i18n("On-screen keyboard for Plasma"),
        KAboutLicense::GPL_V3);
    about.addAuthor(QStringLiteral("DevL0rd"), QString(), QStringLiteral("dmhzmxn@gmail.com"));
    about.setDesktopFileName(QStringLiteral("org.devl0rd.kboard"));
    KAboutData::setApplicationData(about);
    application.setWindowIcon(QIcon::fromTheme(QStringLiteral("input-keyboard-virtual")));
    KCrash::initialize();

    if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE")) {
        QQuickStyle::setStyle(QStringLiteral("org.kde.desktop"));
    }

    SettingsWatcher::startWatching();

    auto bus = QDBusConnection::sessionBus();
    bus.registerObject(QStringLiteral("/KBoard"), KeyboardService::instance(), QDBusConnection::ExportScriptableContents);
    bus.registerService(QStringLiteral("org.devl0rd.KBoard"));

    ActiveAppWatcher activeApp;
    activeApp.start();

    QQmlApplicationEngine engine;
    KBoardQml::loadModule(engine, QStringLiteral("org.devl0rd.kboard"), QStringLiteral("Main"));
    return application.exec();
}
