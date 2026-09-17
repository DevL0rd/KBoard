#include "keyboardservice.h"
#include "settingsbridge.h"

#include <kboard_version.h>

#include <KAboutData>
#include <KCrash>
#include <KLocalizedQmlContext>
#include <KLocalizedString>

#include <QDBusConnection>
#include <QDir>
#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQuickStyle>

int main(int argc, char **argv)
{
    qunsetenv("QT_IM_MODULE");
    QGuiApplication application(argc, argv);
    KLocalizedString::setApplicationDomain("kboard");

    KAboutData about(QStringLiteral("kboard"), i18n("KBoard"), QStringLiteral(KBOARD_VERSION_STRING), i18n("On-screen keyboard for Plasma"), KAboutLicense::GPL_V3);
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

    QQmlApplicationEngine engine;
    const QString buildQml = QStringLiteral(KBOARD_QML_BUILD_DIR);
    if (QDir(buildQml).exists() && qEnvironmentVariableIsSet("KBOARD_USE_BUILD_TREE")) {
        engine.addImportPath(buildQml);
    } else {
        engine.addImportPath(QStringLiteral(KBOARD_QML_INSTALL_DIR));
    }
    KLocalization::setupLocalizedContext(&engine);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed, &application, [] {
        QCoreApplication::exit(1);
    }, Qt::QueuedConnection);
    engine.loadFromModule("org.devl0rd.kboard", "Main");
    return application.exec();
}
