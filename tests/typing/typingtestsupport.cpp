#include "typingtestsupport.h"

#include "kboardsettings.h"
#include "typingengine.h"

#include <QDir>
#include <QPointF>
#include <QRandomGenerator>
#include <QStandardPaths>
#include <QTest>

namespace TypingTest
{
namespace
{
constexpr double Jitter = 0.14;
constexpr double StepPixels = 6.0;
constexpr int SmoothingPasses = 1;
constexpr int ReadyTimeoutMs = 20000;

const QStringList &rows()
{
    static const QStringList value = {QStringLiteral("qwertyuiop"), QStringLiteral("asdfghjkl"), QStringLiteral("zxcvbnm")};
    return value;
}

QPointF centreOf(QChar letter)
{
    const double offsets[] = {0.0, 0.5, 1.5};
    for (int row = 0; row < rows().size(); ++row) {
        const qsizetype column = rows().at(row).indexOf(letter);
        if (column >= 0) {
            return QPointF((offsets[row] + double(column) + 0.5) * KeyWidth, (row + 0.5) * KeyHeight);
        }
    }
    return QPointF();
}

QVector<QPointF> chaikin(const QVector<QPointF> &points)
{
    QVector<QPointF> smoothed;
    smoothed.append(points.first());
    for (int i = 0; i + 1 < points.size(); ++i) {
        smoothed.append(points[i] * 0.75 + points[i + 1] * 0.25);
        smoothed.append(points[i] * 0.25 + points[i + 1] * 0.75);
    }
    smoothed.append(points.last());
    return smoothed;
}
}

void prepareEnvironment()
{
    QStandardPaths::setTestModeEnabled(true);
    qputenv("KBOARD_USE_BUILD_TREE", "1");
    QDir(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/kboard/typing")).removeRecursively();
    resetSettings();
}

void resetSettings()
{
    KBoardSettings::self()->setDefaults();
}

std::unique_ptr<TypingEngine> readyEngine()
{
    auto engine = std::make_unique<TypingEngine>();
    engine->setLayout(qwertyLayout());
    const bool ready = QTest::qWaitFor([&engine] { return engine->isReady() && engine->isGlideReady(); }, ReadyTimeoutMs);
    if (!ready) {
        qWarning("TypingEngine did not become ready: %s", qPrintable(engine->errorString()));
    }
    return engine;
}

QVariantList qwertyLayout()
{
    QVariantList keys;
    for (const QString &row : rows()) {
        for (QChar letter : row) {
            const QPointF centre = centreOf(letter);
            keys.append(QVariantMap {
                {QStringLiteral("label"), QString(letter)},
                {QStringLiteral("x"), centre.x() - KeyWidth / 2},
                {QStringLiteral("y"), centre.y() - KeyHeight / 2},
                {QStringLiteral("width"), KeyWidth},
                {QStringLiteral("height"), KeyHeight},
            });
        }
    }
    keys.append(QVariantMap {{QStringLiteral("label"), QStringLiteral("space")}, {QStringLiteral("x"), 250.0},
        {QStringLiteral("y"), 3 * KeyHeight}, {QStringLiteral("width"), 480.0}, {QStringLiteral("height"), KeyHeight}});
    return keys;
}

QVariantList glidePath(const QString &word, quint32 seed)
{
    QRandomGenerator random(seed);
    QVector<QPointF> vertices;
    for (QChar letter : word) {
        const QPointF jitter(
            (random.generateDouble() - 0.5) * 2 * Jitter * KeyWidth, (random.generateDouble() - 0.5) * 2 * Jitter * KeyHeight);
        vertices.append(centreOf(letter) + jitter);
    }
    for (int pass = 0; pass < SmoothingPasses && vertices.size() > 2; ++pass) {
        vertices = chaikin(vertices);
    }
    QVariantList path;
    for (int i = 0; i + 1 < vertices.size(); ++i) {
        const QPointF delta = vertices[i + 1] - vertices[i];
        const int steps = std::max(1, int(std::hypot(delta.x(), delta.y()) / StepPixels));
        for (int step = 0; step < steps; ++step) {
            path.append(vertices[i] + delta * (double(step) / steps));
        }
    }
    path.append(vertices.last());
    return path;
}

void typeWord(TypingEngine &engine, QString &text, const QString &word)
{
    for (QChar c : word) {
        text.append(c);
        engine.update(text, QString());
    }
}
}
