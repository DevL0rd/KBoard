#include "typingengine.h"
#include "typingtestsupport.h"

#include <QElapsedTimer>
#include <QTest>

class PerformanceTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void updateBudget();
    void correctionBudget();

private:
    std::unique_ptr<TypingEngine> m_engine;
};

void PerformanceTest::initTestCase()
{
    TypingTest::prepareEnvironment();
    m_engine = TypingTest::readyEngine();
    QVERIFY(m_engine->isReady());
    QVERIFY(m_engine->isGlideReady());
}

void PerformanceTest::updateBudget()
{
    const QString paragraph
        = QStringLiteral("Hey techincal team, I wanted to let you know that the deployment tomorow morning is postponed becuase "
                         "the integration tests keep failing on the staging servers. Please recieve this update calmly and "
                         "tell everyone who is responsible for the infrastructure dashboards");
    QString text;
    int calls = 0;
    qint64 worst = 0;
    QElapsedTimer total;
    total.start();
    for (QChar c : paragraph) {
        text.append(c);
        QElapsedTimer timer;
        timer.start();
        m_engine->update(text, QString());
        worst = std::max(worst, timer.nsecsElapsed());
        ++calls;
    }
    const double average = double(total.nsecsElapsed()) / calls / 1e6;
    qInfo("update() average %.3f ms, worst %.3f ms over %d keystrokes", average, double(worst) / 1e6, calls);
    QVERIFY(average < 5.0);
}

void PerformanceTest::correctionBudget()
{
    const QStringList typos = QStringLiteral("teh helo recieve becuase definately seperate occured tomorow wierd accomodation "
                                             "enviroment goverment independant neccessary")
                                  .split(u' ');
    QElapsedTimer timer;
    timer.start();
    for (const QString &typo : typos) {
        QVERIFY(!m_engine->correctionFor(typo).isEmpty());
    }
    const double average = double(timer.nsecsElapsed()) / typos.size() / 1e6;
    qInfo("correctionFor() average %.3f ms", average);
    QVERIFY(average < 5.0);
}

QTEST_GUILESS_MAIN(PerformanceTest)
#include "performancetest.moc"
