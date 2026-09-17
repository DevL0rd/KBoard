#include "keysound.h"

#include "kboardpaths.h"
#include "kboardsettings.h"

#include <QCommandLineParser>
#include <QGuiApplication>
#include <QTextStream>
#include <QTimer>

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addOption({QStringLiteral("pack"), QStringLiteral("Sound pack to use"), QStringLiteral("id"), QStringLiteral("soft")});
    parser.addOption({QStringLiteral("presses"), QStringLiteral("Number of key presses"), QStringLiteral("count"), QStringLiteral("40")});
    parser.addOption(
        {QStringLiteral("interval"), QStringLiteral("Milliseconds between presses"), QStringLiteral("ms"), QStringLiteral("90")});
    parser.addOption({QStringLiteral("preview"), QStringLiteral("Play the preview sequence of every pack")});
    parser.process(app);

    KeySound sound(KeySound::Output::Device, KBoardPaths::dataFile(QStringLiteral("sounds")));
    QTextStream out(stdout);
    QObject::connect(&sound, &KeySound::deviceChanged, &app,
        [&] { out << "device open=" << sound.deviceOpen() << " name=" << sound.deviceName() << Qt::endl; });
    QObject::connect(&sound, &KeySound::errorStringChanged, &app, [&] { out << "error: " << sound.errorString() << Qt::endl; });
    sound.setActive(true);

    if (parser.isSet(QStringLiteral("preview"))) {
        const QVariantList packs = sound.packs();
        for (int i = 0; i < packs.size(); ++i) {
            const QString id = packs[i].toMap().value(QStringLiteral("id")).toString();
            QTimer::singleShot(300 + i * 1800, &app, [&sound, &out, id] {
                out << "preview " << id << Qt::endl;
                sound.preview(id);
            });
        }
        QTimer::singleShot(300 + packs.size() * 1800 + 500, &app, &QCoreApplication::quit);
        return app.exec();
    }

    KBoardSettings::setSoundPack(parser.value(QStringLiteral("pack")));
    const int presses = parser.value(QStringLiteral("presses")).toInt();
    const int interval = parser.value(QStringLiteral("interval")).toInt();
    auto timer = new QTimer(&app);
    int count = 0;
    QObject::connect(timer, &QTimer::timeout, &app, [&] {
        static const QStringList kinds {QStringLiteral("key"), QStringLiteral("key"), QStringLiteral("key"), QStringLiteral("space")};
        sound.play(kinds[count % kinds.size()]);
        if (++count >= presses) {
            timer->stop();
            QTimer::singleShot(400, &app, [&] {
                out << "latencyMs=" << sound.latencyMs() << " bufferFrames=" << sound.bufferFrames()
                    << " queueDelayMs=" << sound.mixer().queueDelayMs() << " blockMs=" << sound.mixer().blockMs() << Qt::endl;
                QCoreApplication::quit();
            });
        }
    });
    QTimer::singleShot(500, timer, [timer, interval] { timer->start(interval); });
    return app.exec();
}
