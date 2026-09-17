#include "gamepad.h"
#include "kboardsettings.h"

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QSocketNotifier>
#include <QStandardPaths>
#include <QTextStream>

#include <unistd.h>

int main(int argc, char **argv)
{
    QStandardPaths::setTestModeEnabled(true);
    QCoreApplication app(argc, argv);
    KBoardSettings::self()->setDefaults();

    QTextStream out(stdout);
    QElapsedTimer clock;
    clock.start();
    auto emitLine = [&out, &clock](const QString &line) { out << clock.elapsed() << ' ' << line << Qt::endl; };

    auto *gamepad = Gamepad::instance();
    QObject::connect(gamepad, &Gamepad::availableChanged, [&] { emitLine(QStringLiteral("available %1").arg(gamepad->available())); });
    QObject::connect(gamepad, &Gamepad::errorStringChanged, [&] { emitLine(QStringLiteral("error %1").arg(gamepad->errorString())); });
    QObject::connect(gamepad, &Gamepad::warningsChanged, [&] {
        if (!gamepad->warnings().isEmpty()) {
            emitLine(QStringLiteral("warning %1").arg(gamepad->warnings().constLast()));
        }
    });
    QObject::connect(gamepad, &Gamepad::devicesChanged, [&] {
        QStringList parts;
        for (const QVariant &entry : gamepad->devices()) {
            const QVariantMap map = entry.toMap();
            parts.append(QStringLiteral("%1:%2:%3:virtual=%4:ignored=%5:active=%6:rumble=%7")
                    .arg(map.value(QStringLiteral("id")).toInt())
                    .arg(map.value(QStringLiteral("name")).toString().replace(QLatin1Char(' '), QLatin1Char('_')))
                    .arg(map.value(QStringLiteral("controllerType")).toString())
                    .arg(map.value(QStringLiteral("steamVirtual")).toBool())
                    .arg(map.value(QStringLiteral("ignored")).toBool())
                    .arg(map.value(QStringLiteral("active")).toBool())
                    .arg(map.value(QStringLiteral("rumble")).toBool()));
        }
        emitLine(QStringLiteral("devices %1").arg(parts.join(QLatin1Char(' '))));
    });
    QObject::connect(gamepad, &Gamepad::activeDeviceChanged, [&] {
        emitLine(QStringLiteral("active %1 %2 %3 glyph-a=%4")
                .arg(gamepad->activeDeviceId())
                .arg(gamepad->controllerType(), gamepad->name().replace(QLatin1Char(' '), QLatin1Char('_')),
                    gamepad->glyph(QStringLiteral("a"))));
    });
    QObject::connect(gamepad, &Gamepad::buttonPressed, [&](const QString &button) { emitLine(QStringLiteral("pressed %1").arg(button)); });
    QObject::connect(
        gamepad, &Gamepad::buttonReleased, [&](const QString &button) { emitLine(QStringLiteral("released %1").arg(button)); });
    QObject::connect(gamepad, &Gamepad::buttonTapped, [&](const QString &button) { emitLine(QStringLiteral("tapped %1").arg(button)); });
    QObject::connect(gamepad, &Gamepad::navigate, [&](int dx, int dy) { emitLine(QStringLiteral("navigate %1 %2").arg(dx).arg(dy)); });
    QObject::connect(gamepad, &Gamepad::chordActivated, [&] { emitLine(QStringLiteral("chord")); });
    QObject::connect(gamepad, &Gamepad::rightStickChanged, [&] {
        emitLine(QStringLiteral("rightstick %1 %2 angle=%3")
                .arg(gamepad->rightStick().x(), 0, 'f', 2)
                .arg(gamepad->rightStick().y(), 0, 'f', 2)
                .arg(gamepad->rightStickAngle(), 0, 'f', 0));
    });
    QObject::connect(
        gamepad, &Gamepad::leftTriggerChanged, [&] { emitLine(QStringLiteral("lefttrigger %1").arg(gamepad->leftTrigger(), 0, 'f', 2)); });

    QSocketNotifier input(STDIN_FILENO, QSocketNotifier::Read);
    QTextStream in(stdin);
    QObject::connect(&input, &QSocketNotifier::activated, [&] {
        QString line;
        if (!in.readLineInto(&line)) {
            app.quit();
            return;
        }
        const QStringList words = line.split(QLatin1Char(' '), Qt::SkipEmptyParts);
        if (words.isEmpty()) {
            return;
        }
        const QString &command = words.first();
        if (command == QLatin1String("quit")) {
            app.quit();
        } else if (command == QLatin1String("rumble") && words.size() == 3) {
            emitLine(QStringLiteral("rumble-sent %1").arg(gamepad->rumble(words.at(1).toDouble(), words.at(2).toInt())));
        } else if (command == QLatin1String("enabled") && words.size() == 2) {
            KBoardSettings::self()->setControllerEnabled(words.at(1) == QLatin1String("1"));
            emitLine(QStringLiteral("enabled %1").arg(gamepad->enabled()));
        } else if (command == QLatin1String("speed") && words.size() == 2) {
            KBoardSettings::self()->setControllerStickSpeed(words.at(1).toDouble());
        } else if (command == QLatin1String("ping")) {
            emitLine(QStringLiteral("pong"));
        }
    });

    return app.exec();
}
