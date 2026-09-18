#include "aboutinfo.h"
#include "appnavigation.h"
#include "qmlengine.h"
#include "settingscatalog.h"

#include <KAboutData>
#include <KCrash>
#include <KDBusService>
#include <KLocalizedString>
#include <KWindowSystem>

#include <QApplication>
#include <QCommandLineParser>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QQuickWindow>

namespace
{
const QString PageOption = QStringLiteral("page");
const QString RevealOption = QStringLiteral("reveal");
const QString SearchOption = QStringLiteral("search");

void addNavigationOptions(QCommandLineParser &parser)
{
    parser.addOption(QCommandLineOption(PageOption, i18n("Open this settings page"), QStringLiteral("id")));
    parser.addOption(QCommandLineOption(RevealOption, i18n("Scroll to and highlight this row on the page"), QStringLiteral("label")));
    parser.addOption(QCommandLineOption(SearchOption, i18n("Start with this text in the search field"), QStringLiteral("text")));
}

void navigate(const QCommandLineParser &parser)
{
    if (parser.isSet(SearchOption)) {
        AppNavigation::instance()->setSearchText(parser.value(SearchOption));
    }
    if (parser.isSet(PageOption)) {
        AppNavigation::instance()->open(parser.value(PageOption), parser.value(RevealOption));
    }
}

bool pageIsKnown(const QCommandLineParser &parser)
{
    const SettingsCatalog catalog;
    if (!parser.isSet(PageOption) || catalog.hasPage(parser.value(PageOption))) {
        return true;
    }
    QStringList ids;
    const QVariantList pages = catalog.pages();
    for (const QVariant &page : pages) {
        ids.append(page.toMap().value(QStringLiteral("id")).toString());
    }
    qCritical().noquote() << "Unknown page" << parser.value(PageOption) << "- pages are:" << ids.join(QStringLiteral(", "));
    return false;
}

void loadInterface(QQmlApplicationEngine &engine)
{
    KBoardQml::loadModule(engine, QStringLiteral("org.devl0rd.kboard.settings"), QStringLiteral("Main"));
}

void raiseWindows(const QQmlApplicationEngine &engine)
{
    const auto roots = engine.rootObjects();
    for (QObject *root : roots) {
        auto window = qobject_cast<QQuickWindow *>(root);
        if (!window) {
            continue;
        }
        window->show();
        window->setWindowStates(window->windowStates() & ~Qt::WindowMinimized);
        KWindowSystem::updateStartupId(window);
        KWindowSystem::activateWindow(window);
    }
}

void handleActivation(const QQmlApplicationEngine &engine, const QStringList &arguments)
{
    QCommandLineParser remote;
    addNavigationOptions(remote);
    if (!arguments.isEmpty() && remote.parse(arguments)) {
        navigate(remote);
    }
    raiseWindows(engine);
}
}

int main(int argc, char **argv)
{
    qunsetenv("QT_IM_MODULE");
    QApplication application(argc, argv);
    AboutInfo::install();
    KAboutData about = KAboutData::applicationData();
    QApplication::setWindowIcon(QIcon::fromTheme(QStringLiteral("input-keyboard-virtual")));
    KCrash::initialize();
    if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE")) {
        QQuickStyle::setStyle(QStringLiteral("org.kde.desktop"));
    }

    QCommandLineParser parser;
    addNavigationOptions(parser);
    about.setupCommandLine(&parser);
    parser.process(application);
    about.processCommandLine(&parser);
    if (!pageIsKnown(parser)) {
        return 2;
    }

    KDBusService service(KDBusService::Unique);
    navigate(parser);

    QQmlApplicationEngine engine;
    loadInterface(engine);
    QObject::connect(&service, &KDBusService::activateRequested, &application,
        [&engine](const QStringList &arguments, const QString &) { handleActivation(engine, arguments); });
    return QApplication::exec();
}
