#include "aboutinfo.h"
#include "appnavigation.h"
#include "settingscatalog.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QEventLoop>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QTimer>

namespace
{
void wait(int ms)
{
    QEventLoop loop;
    QTimer::singleShot(ms, &loop, &QEventLoop::quit);
    loop.exec();
}

QStringList pageIds(const QString &requested)
{
    if (!requested.isEmpty()) {
        return requested.split(QLatin1Char(','));
    }
    QStringList ids;
    const SettingsCatalog catalog;
    for (const QVariant &page : catalog.pages()) {
        ids.append(page.toMap().value(QStringLiteral("id")).toString());
    }
    return ids;
}
}

int main(int argc, char **argv)
{
    QApplication application(argc, argv);
    QQuickStyle::setStyle(QStringLiteral("org.kde.desktop"));
    AboutInfo::install();

    QCommandLineParser parser;
    parser.addOption({QStringLiteral("out"), QStringLiteral("Output directory"), QStringLiteral("dir")});
    parser.addOption({QStringLiteral("pages"), QStringLiteral("Comma separated page ids"), QStringLiteral("ids")});
    parser.addOption({QStringLiteral("suffix"), QStringLiteral("File name suffix"), QStringLiteral("text")});
    parser.addOption({QStringLiteral("delay"), QStringLiteral("Milliseconds to settle"), QStringLiteral("ms"), QStringLiteral("2200")});
    parser.addOption({QStringLiteral("size"), QStringLiteral("Window size WxH"), QStringLiteral("size"), QStringLiteral("1200x860")});
    parser.addOption({QStringLiteral("reveal"), QStringLiteral("Row label to reveal"), QStringLiteral("label")});
    parser.addOption({QStringLiteral("search"), QStringLiteral("Text to put in the search field"), QStringLiteral("text")});
    parser.process(application);

    QQmlApplicationEngine engine;
    engine.addImportPath(QStringLiteral(KBOARD_QML_BUILD_DIR));
    engine.loadFromModule("org.devl0rd.kboard.settings", "Main");
    if (engine.rootObjects().isEmpty()) {
        return 1;
    }
    auto window = qobject_cast<QQuickWindow *>(engine.rootObjects().constFirst());
    const QStringList size = parser.value(QStringLiteral("size")).split(QLatin1Char('x'));
    window->resize(size.value(0).toInt(), size.value(1).toInt());
    window->show();

    const QString out = parser.value(QStringLiteral("out"));
    QDir().mkpath(out);
    const int delay = parser.value(QStringLiteral("delay")).toInt();
    const QStringList ids = pageIds(parser.value(QStringLiteral("pages")));
    AppNavigation::instance()->setSearchText(parser.value(QStringLiteral("search")));
    for (const QString &id : ids) {
        AppNavigation::instance()->open(id, parser.value(QStringLiteral("reveal")));
        wait(delay);
        const QString path = QStringLiteral("%1/settings-%2%3.png").arg(out, id, parser.value(QStringLiteral("suffix")));
        if (!window->grabWindow().save(path)) {
            qCritical() << "Couldn't save" << path;
            return 1;
        }
        qInfo().noquote() << path;
    }
    return 0;
}
