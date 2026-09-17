#include "inputcontext.h"
#include "kboardsettings.h"
#include "typingengine.h"
#include "typingtestsupport.h"

#include <QTemporaryDir>
#include <QTest>

class PredictionTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void init();
    void completions();
    void completionCasing();
    void suggestionsWhileTyping();
    void basePredictions();
    void learnsPhrases();
    void learningRespectsSettings();
    void forgetBlocksWord();
    void persistsLearnedWords();
    void exportImport();

private:
    void typeSentence(const QString &sentence);

    std::unique_ptr<TypingEngine> m_engine;
};

void PredictionTest::initTestCase()
{
    TypingTest::prepareEnvironment();
    m_engine = TypingTest::readyEngine();
    QVERIFY(m_engine->isReady());
    QVERIFY(m_engine->isGlideReady());
}

void PredictionTest::init()
{
    TypingTest::resetSettings();
    m_engine->clearLearned();
    m_engine->setSensitive(false);
    m_engine->setContentPurpose(InputContext::content_purpose_normal);
    m_engine->update(QString(), QString());
}

void PredictionTest::typeSentence(const QString &sentence)
{
    QString text;
    const QStringList words = sentence.split(u' ');
    for (const QString &word : words) {
        TypingTest::typeWord(*m_engine, text, word);
        m_engine->acceptWord(word);
        text.append(u' ');
        m_engine->update(text, QString());
    }
}

void PredictionTest::completions()
{
    QCOMPARE(m_engine->completionsFor(QStringLiteral("tomor")).value(0), QStringLiteral("tomorrow"));
    QVERIFY(m_engine->completionsFor(QStringLiteral("hel")).contains(QStringLiteral("hello")));
    QCOMPARE(m_engine->completionsFor(QStringLiteral("hel"), 2).size(), 2);
    QVERIFY(m_engine->completionsFor(QString()).isEmpty());
}

void PredictionTest::completionCasing()
{
    QCOMPARE(m_engine->completionsFor(QStringLiteral("Tomor")).value(0), QStringLiteral("Tomorrow"));
    QCOMPARE(m_engine->completionsFor(QStringLiteral("TOMOR")).value(0), QStringLiteral("TOMORROW"));
    QCOMPARE(m_engine->completionsFor(QStringLiteral("engl")).value(0), QStringLiteral("English"));
}

void PredictionTest::suggestionsWhileTyping()
{
    m_engine->update(QStringLiteral("see you tomor"), QString());
    QCOMPARE(m_engine->currentWord(), QStringLiteral("tomor"));
    QCOMPARE(m_engine->suggestions().value(1), QStringLiteral("tomorrow"));
    QCOMPARE(m_engine->autocorrection(), QString());
    const QVariantList items = m_engine->suggestionItems();
    QCOMPARE(items.size(), 3);
    QCOMPARE(items.at(1).toMap().value(QStringLiteral("kind")).toString(), QStringLiteral("completion"));
}

void PredictionTest::basePredictions()
{
    QVERIFY(m_engine->predictNext(QStringLiteral("thank ")).contains(QStringLiteral("you")));
    QVERIFY(!m_engine->predictNext(QStringLiteral("I want to ")).isEmpty());
    m_engine->update(QStringLiteral("How are "), QString());
    QCOMPARE(m_engine->suggestions().value(1), QStringLiteral("you"));
    m_engine->update(QStringLiteral("Hi. "), QString());
    const QString first = m_engine->suggestions().value(1);
    QVERIFY(!first.isEmpty());
    QVERIFY(first.at(0).isUpper());
    KBoardSettings::self()->setNextWordPrediction(false);
    QVERIFY(m_engine->predictNext(QStringLiteral("thank ")).isEmpty());
}

void PredictionTest::learnsPhrases()
{
    for (int i = 0; i < 3; ++i) {
        typeSentence(QStringLiteral("we deploy kboardix tonight"));
    }
    QVERIFY(m_engine->isLearned(QStringLiteral("kboardix")));
    QVERIFY(m_engine->learnedWords().contains(QStringLiteral("kboardix")));
    QCOMPARE(m_engine->predictNext(QStringLiteral("we deploy ")).value(0), QStringLiteral("kboardix"));
    m_engine->update(QStringLiteral("so we deploy "), QString());
    QCOMPARE(m_engine->suggestions().value(1), QStringLiteral("kboardix"));
    QCOMPARE(m_engine->completionsFor(QStringLiteral("kboa")).value(0), QStringLiteral("kboardix"));
    m_engine->update(QStringLiteral("kboardx"), QString());
    QCOMPARE(m_engine->autocorrection(), QStringLiteral("kboardix"));
}

void PredictionTest::learningRespectsSettings()
{
    KBoardSettings::self()->setLearnWords(false);
    typeSentence(QStringLiteral("zorblat zorblat"));
    typeSentence(QStringLiteral("zorblat zorblat"));
    QVERIFY(!m_engine->isLearned(QStringLiteral("zorblat")));
    KBoardSettings::self()->setLearnWords(true);
    m_engine->setSensitive(true);
    typeSentence(QStringLiteral("zorblat zorblat"));
    QVERIFY(!m_engine->isLearned(QStringLiteral("zorblat")));
    m_engine->setSensitive(false);
    m_engine->setContentPurpose(InputContext::content_purpose_terminal);
    typeSentence(QStringLiteral("zorblat zorblat"));
    QVERIFY(!m_engine->isLearned(QStringLiteral("zorblat")));
}

void PredictionTest::forgetBlocksWord()
{
    QVERIFY(m_engine->completionsFor(QStringLiteral("tomor")).contains(QStringLiteral("tomorrow")));
    m_engine->forget(QStringLiteral("tomorrow"));
    QVERIFY(!m_engine->completionsFor(QStringLiteral("tomor")).contains(QStringLiteral("tomorrow")));
    m_engine->learn(QStringLiteral("tomorrow"));
    QVERIFY(m_engine->completionsFor(QStringLiteral("tomor")).contains(QStringLiteral("tomorrow")));
}

void PredictionTest::persistsLearnedWords()
{
    m_engine->learn(QStringLiteral("Plasmashellix"));
    m_engine->flush();
    auto second = TypingTest::readyEngine();
    QVERIFY(second->isReady());
    QVERIFY(second->isLearned(QStringLiteral("plasmashellix")));
    QCOMPARE(second->completionsFor(QStringLiteral("plasmash")).value(0), QStringLiteral("Plasmashellix"));
}

void PredictionTest::exportImport()
{
    QTemporaryDir dir;
    const QString path = dir.filePath(QStringLiteral("words.txt"));
    m_engine->learn(QStringLiteral("Konveyorix"));
    QVERIFY(m_engine->exportLearned(path));
    m_engine->clearLearned();
    QVERIFY(!m_engine->isLearned(QStringLiteral("konveyorix")));
    QCOMPARE(m_engine->importLearned(path), 1);
    QVERIFY(m_engine->isLearned(QStringLiteral("konveyorix")));
}

QTEST_GUILESS_MAIN(PredictionTest)
#include "predictiontest.moc"
