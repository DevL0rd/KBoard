#include "kboardsettings.h"
#include "typingengine.h"
#include "typingtestsupport.h"

#include <QElapsedTimer>
#include <QSignalSpy>
#include <QTest>

class GlideTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void init();
    void topOneAccuracy();
    void capitalisesAtSentenceStart();
    void asyncDecode();
    void respectsSetting();
    void decodeBudget();

private:
    static QStringList commonWords();

    std::unique_ptr<TypingEngine> m_engine;
};

QStringList GlideTest::commonWords()
{
    return QStringLiteral(
        "the and you that was for are with they this have from one had word but not what all were when your can said there "
        "each which she how their will other about out many then them these some her would make like him into time has look two "
        "more write see number way could people than first water been call who now find long down day did get come made may part "
        "hello keyboard please thanks tomorrow because question between important something weekend together morning")
        .split(u' ');
}

void GlideTest::initTestCase()
{
    TypingTest::prepareEnvironment();
    m_engine = TypingTest::readyEngine();
    QVERIFY(m_engine->isReady());
    QVERIFY(m_engine->isGlideReady());
}

void GlideTest::init()
{
    TypingTest::resetSettings();
    m_engine->update(QStringLiteral("so "), QString());
}

void GlideTest::topOneAccuracy()
{
    const QStringList words = commonWords();
    int total = 0;
    int topOne = 0;
    int topThree = 0;
    QStringList misses;
    for (const QString &word : words) {
        for (quint32 seed = 1; seed <= 3; ++seed) {
            const QStringList candidates = m_engine->decodeGlide(TypingTest::glidePath(word, seed * 7919u + quint32(word.size())));
            ++total;
            topOne += candidates.value(0) == word;
            topThree += candidates.mid(0, 3).contains(word);
            if (candidates.value(0) != word) {
                misses.append(word + QStringLiteral("->") + candidates.value(0));
            }
        }
    }
    const double accuracy = double(topOne) / total;
    qInfo("glide top-1 %.1f%% top-3 %.1f%% over %d paths; misses: %s", accuracy * 100, double(topThree) * 100 / total, total,
        qPrintable(misses.join(u' ')));
    QVERIFY2(accuracy >= 0.9, qPrintable(misses.join(u' ')));
    QVERIFY(double(topThree) / total >= 0.97);
}

void GlideTest::capitalisesAtSentenceStart()
{
    m_engine->update(QStringLiteral("Done. "), QString());
    QCOMPARE(m_engine->decodeGlide(TypingTest::glidePath(QStringLiteral("hello"), 3)).value(0), QStringLiteral("Hello"));
}

void GlideTest::asyncDecode()
{
    QSignalSpy spy(m_engine.get(), &TypingEngine::glideDecoded);
    m_engine->decodeGlideAsync(TypingTest::glidePath(QStringLiteral("keyboard"), 5));
    QVERIFY(spy.wait(5000));
    QCOMPARE(spy.first().first().toStringList().value(0), QStringLiteral("keyboard"));
}

void GlideTest::respectsSetting()
{
    KBoardSettings::self()->setGlideTyping(false);
    QVERIFY(m_engine->decodeGlide(TypingTest::glidePath(QStringLiteral("hello"), 3)).isEmpty());
}

void GlideTest::decodeBudget()
{
    const QStringList words = commonWords();
    QList<QVariantList> paths;
    for (const QString &word : words) {
        paths.append(TypingTest::glidePath(word, 11));
    }
    qint64 worst = 0;
    QElapsedTimer total;
    total.start();
    for (const QVariantList &path : std::as_const(paths)) {
        QElapsedTimer timer;
        timer.start();
        m_engine->decodeGlide(path);
        worst = std::max(worst, timer.nsecsElapsed());
    }
    const double average = double(total.nsecsElapsed()) / paths.size() / 1e6;
    qInfo("glide decode average %.2f ms, worst %.2f ms", average, double(worst) / 1e6);
    QVERIFY(average < 30.0);
}

QTEST_GUILESS_MAIN(GlideTest)
#include "glidetest.moc"
