#include "clipboardhistory.h"
#include "kboardsettings.h"

#include <KLocalizedQmlContext>
#include <KLocalizedString>

#include <QCommandLineParser>
#include <QCursor>
#include <QGuiApplication>
#include <QLinearGradient>
#include <QPainter>
#include <QPainterPath>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QTemporaryDir>
#include <QTimer>

namespace
{
QImage artwork(const QSize &size, const QColor &from, const QColor &to, int seed)
{
    QImage image(size, QImage::Format_ARGB32_Premultiplied);
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    QLinearGradient gradient(0, 0, size.width(), size.height());
    gradient.setColorAt(0, from);
    gradient.setColorAt(1, to);
    painter.fillRect(image.rect(), gradient);
    painter.setPen(Qt::NoPen);
    for (int i = 0; i < 6; ++i) {
        QColor blob = i % 2 ? QColor(255, 255, 255, 60) : QColor(0, 0, 0, 40);
        painter.setBrush(blob);
        const int r = size.height() / (3 + (i + seed) % 3);
        painter.drawEllipse(QPoint((i * 97 + seed * 53) % size.width(), (i * 61 + seed * 31) % size.height()), r, r);
    }
    QPainterPath hills;
    hills.moveTo(0, size.height());
    hills.cubicTo(size.width() * 0.3, size.height() * 0.45, size.width() * 0.6, size.height() * 0.95, size.width(), size.height() * 0.6);
    hills.lineTo(size.width(), size.height());
    painter.setBrush(QColor(0, 0, 0, 70));
    painter.drawPath(hills);
    return image;
}

struct Sample
{
    qint64 secondsAgo;
    QString text;
    QImage image;
    bool pin;
};

QList<Sample> samples()
{
    const QString thanks
        = QStringLiteral("Thanks for the quick turnaround! I pushed the fixes to the branch and left a few notes inline. "
                         "Let me know if the new layout reads better on the Steam Deck, especially the split halves in portrait mode.");
    return {
        {3570, QStringLiteral("Soon to expire: the meeting notes from Tuesday"), {}, false},
        {2400, QStringLiteral("Wi-Fi password for the studio is on the fridge, ask Sam"), {}, false},
        {2400, {}, artwork(QSize(1280, 720), QColor(0x3d, 0xae, 0xe9), QColor(0x8e, 0x44, 0xad), 1), false},
        {2400, QStringLiteral("https://invent.kde.org/plasma/kwin/-/merge_requests/7342"), {}, true},
        {1200, thanks, {}, false},
        {1200, QStringLiteral("123 Harbour Street\nApt 4B\nPortland, OR 97205"), {}, true},
        {1200, {}, artwork(QSize(600, 900), QColor(0xf6, 0x74, 0x00), QColor(0xda, 0x44, 0x53), 4), false},
        {1200, QStringLiteral("www.archlinux.org/packages/extra/x86_64/whisper-cpp"), {}, false},
        {120, QStringLiteral("sudo pacman -S --needed kguiaddons"), {}, false},
        {0, QStringLiteral("G-551204 is your Google verification code."), {}, false},
    };
}

void populate(ClipboardHistory *history, QDateTime &clock, const QDateTime &base)
{
    const bool wasPaused = KBoardSettings::clipboardPaused();
    KBoardSettings::setClipboardPaused(false);
    for (const Sample &sample : samples()) {
        clock = base.addSecs(-sample.secondsAgo);
        const bool added = sample.image.isNull() ? history->addText(sample.text) : history->addImage(sample.image);
        if (added && sample.pin) {
            history->pin(0, true);
        }
    }
    clock = base;
    history->expireDue();
    KBoardSettings::setClipboardPaused(wasPaused);
}

void addOptions(QCommandLineParser &parser)
{
    parser.addHelpOption();
    parser.addOption({QStringLiteral("output"), QStringLiteral("PNG to write, then quit"), QStringLiteral("path")});
    parser.addOption({QStringLiteral("scenario"), QStringLiteral("full, empty, pinned, links, armed, paused, disabled, focus"),
        QStringLiteral("name"), QStringLiteral("full")});
    parser.addOption({QStringLiteral("width"), QStringLiteral("width"), QStringLiteral("px"), QStringLiteral("1100")});
    parser.addOption({QStringLiteral("height"), QStringLiteral("height"), QStringLiteral("px"), QStringLiteral("420")});
    parser.addOption({QStringLiteral("delay"), QStringLiteral("ms before capture"), QStringLiteral("ms"), QStringLiteral("1600")});
}

void captureLater(QQuickWindow *window, const QString &output, int delay)
{
    QTimer::singleShot(delay, window, [window, output] {
        const QImage shot = window->grabWindow();
        if (shot.isNull() || !shot.save(output)) {
            qCritical() << "failed to capture preview" << output;
            QCoreApplication::exit(2);
            return;
        }
        QCoreApplication::quit();
    });
}
}

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    KLocalizedString::setApplicationDomain("kboard");
    QQuickStyle::setStyle(QStringLiteral("org.kde.desktop"));
    QCommandLineParser parser;
    addOptions(parser);
    parser.process(app);

    const QString scenario = parser.value(QStringLiteral("scenario"));
    KBoardSettings::self()->setDefaults();
    KBoardSettings::setClipboardPaused(scenario == QLatin1String("paused"));
    KBoardSettings::setClipboardEnabled(scenario != QLatin1String("disabled"));

    QTemporaryDir directory;
    const QDateTime base = QDateTime::currentDateTimeUtc();
    QDateTime clock = base;
    ClipboardHistory::Options options {
        directory.filePath(QStringLiteral("data")), directory.filePath(QStringLiteral("cache")), false, {}, [&clock] { return clock; }};
    auto history = new ClipboardHistory(options, &app);
    if (scenario != QLatin1String("empty") && scenario != QLatin1String("disabled")) {
        populate(history, clock, base);
    }
    ClipboardHistory::setInstance(history);

    QCursor::setPos(-1000, -1000);
    QQmlApplicationEngine engine;
    engine.addImportPath(QStringLiteral(KBOARD_QML_BUILD_DIR));
    KLocalization::setupLocalizedContext(&engine);
    engine.rootContext()->setContextProperty(QStringLiteral("previewScenario"), scenario);
    engine.rootContext()->setContextProperty(QStringLiteral("previewWidth"), parser.value(QStringLiteral("width")).toInt());
    engine.rootContext()->setContextProperty(QStringLiteral("previewHeight"), parser.value(QStringLiteral("height")).toInt());
    engine.load(QUrl::fromLocalFile(QStringLiteral(KBOARD_CLIPBOARD_PREVIEW_QML)));
    if (engine.rootObjects().isEmpty()) {
        return 1;
    }
    const QString output = parser.value(QStringLiteral("output"));
    if (!output.isEmpty()) {
        captureLater(
            qobject_cast<QQuickWindow *>(engine.rootObjects().constFirst()), output, parser.value(QStringLiteral("delay")).toInt());
    }
    return app.exec();
}
