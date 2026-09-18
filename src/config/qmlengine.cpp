#include "qmlengine.h"

#include <KLocalizedQmlContext>

#include <QCoreApplication>
#include <QDir>
#include <QQmlApplicationEngine>

namespace KBoardQml
{
void loadModule(QQmlApplicationEngine &engine, const QString &uri, const QString &type)
{
    const QString buildQml = QStringLiteral(KBOARD_QML_BUILD_DIR);
    if (QDir(buildQml).exists() && qEnvironmentVariableIsSet("KBOARD_USE_BUILD_TREE")) {
        engine.addImportPath(buildQml);
    } else {
        engine.addImportPath(QStringLiteral(KBOARD_QML_INSTALL_DIR));
    }
    KLocalization::setupLocalizedContext(&engine);
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, QCoreApplication::instance(), [] { QCoreApplication::exit(1); },
        Qt::QueuedConnection);
    engine.loadFromModule(uri, type);
}
}
