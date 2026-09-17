#include "inputcontext.h"
#include "kboardsettings.h"
#include "typingengine.h"
#include "typingtestsupport.h"

#include <QTest>

class CorrectionTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void init();
    void corrections_data();
    void corrections();
    void protectedTokens_data();
    void protectedTokens();
    void casing_data();
    void casing();
    void validWordsStay();
    void strengthOff();
    void purposesBlockCorrection();
    void autocorrectionOnUpdate();
    void revertRemembersWord();
    void expansions();

private:
    std::unique_ptr<TypingEngine> m_engine;
};

void CorrectionTest::initTestCase()
{
    TypingTest::prepareEnvironment();
    m_engine = TypingTest::readyEngine();
    QVERIFY(m_engine->isReady());
    QVERIFY(m_engine->isGlideReady());
}

void CorrectionTest::init()
{
    TypingTest::resetSettings();
    m_engine->clearLearned();
    m_engine->setContentPurpose(InputContext::content_purpose_normal);
    m_engine->setContentHint(0);
    m_engine->setSensitive(false);
    m_engine->update(QString(), QString());
}

void CorrectionTest::corrections_data()
{
    QTest::addColumn<QString>("typed");
    QTest::addColumn<QString>("expected");
    const char *const pairs[][2] = {
        {"teh", "the"},
        {"helo", "hello"},
        {"recieve", "receive"},
        {"becuase", "because"},
        {"wiht", "with"},
        {"adn", "and"},
        {"thnaks", "thanks"},
        {"definately", "definitely"},
        {"tomorow", "tomorrow"},
        {"dont", "don't"},
        {"im", "I'm"},
        {"cant", "can't"},
        {"yuo", "you"},
        {"whats", "what's"},
        {"seperate", "separate"},
        {"agian", "again"},
        {"freind", "friend"},
        {"beleive", "believe"},
        {"occured", "occurred"},
        {"knwo", "know"},
        {"jsut", "just"},
        {"thier", "their"},
        {"wierd", "weird"},
        {"untill", "until"},
    };
    for (const auto &pair : pairs) {
        QTest::newRow(pair[0]) << QString::fromLatin1(pair[0]) << QString::fromUtf8(pair[1]);
    }
}

void CorrectionTest::corrections()
{
    QFETCH(QString, typed);
    QFETCH(QString, expected);
    QCOMPARE(m_engine->correctionFor(typed), expected);
}

void CorrectionTest::protectedTokens_data()
{
    QTest::addColumn<QString>("typed");
    for (const char *token : {"iPhone", "github.com", "@devl0rd", "#hashtag", "me@example.com", "https://kde.org", "1234", "snake_case",
             "camelCase", "l33t", "v2"}) {
        QTest::newRow(token) << QString::fromLatin1(token);
    }
}

void CorrectionTest::protectedTokens()
{
    QFETCH(QString, typed);
    QCOMPARE(m_engine->correctionFor(typed), QString());
    m_engine->update(QStringLiteral("look at ") + typed, QString());
    QCOMPARE(m_engine->autocorrection(), QString());
}

void CorrectionTest::casing_data()
{
    QTest::addColumn<QString>("typed");
    QTest::addColumn<QString>("expected");
    QTest::newRow("capitalised") << QStringLiteral("Teh") << QStringLiteral("The");
    QTest::newRow("upper") << QStringLiteral("TEH") << QStringLiteral("THE");
    QTest::newRow("pronoun") << QStringLiteral("i") << QStringLiteral("I");
    QTest::newRow("proper noun") << QStringLiteral("english") << QStringLiteral("English");
    QTest::newRow("capitalised typo") << QStringLiteral("Helo") << QStringLiteral("Hello");
    QTest::newRow("contraction") << QStringLiteral("Dont") << QStringLiteral("Don't");
}

void CorrectionTest::casing()
{
    QFETCH(QString, typed);
    QFETCH(QString, expected);
    QCOMPARE(m_engine->correctionFor(typed), expected);
}

void CorrectionTest::validWordsStay()
{
    for (const char *word : {"the", "hello", "its", "shell", "were", "well", "lets", "us", "may", "south", "Hello", "HELLO", "receive",
             "lol", "haha", "ok", "gonna", "plasma"}) {
        QCOMPARE(m_engine->correctionFor(QString::fromLatin1(word)), QString());
    }
}

void CorrectionTest::strengthOff()
{
    KBoardSettings::self()->setAutocorrect(KBoardSettings::EnumAutocorrect::Off);
    QCOMPARE(m_engine->correctionFor(QStringLiteral("teh")), QString());
    KBoardSettings::self()->setAutocorrect(KBoardSettings::EnumAutocorrect::Mild);
    QCOMPARE(m_engine->correctionFor(QStringLiteral("teh")), QStringLiteral("the"));
    KBoardSettings::self()->setAutocorrect(KBoardSettings::EnumAutocorrect::Aggressive);
    QCOMPARE(m_engine->correctionFor(QStringLiteral("recieve")), QStringLiteral("receive"));
}

void CorrectionTest::purposesBlockCorrection()
{
    for (int purpose : {InputContext::content_purpose_terminal, InputContext::content_purpose_password, InputContext::content_purpose_url,
             InputContext::content_purpose_email}) {
        m_engine->setContentPurpose(purpose);
        QCOMPARE(m_engine->correctionFor(QStringLiteral("teh")), QString());
        m_engine->update(QStringLiteral("teh"), QString());
        QCOMPARE(m_engine->autocorrection(), QString());
        QVERIFY(m_engine->suggestions().isEmpty());
    }
    m_engine->setContentPurpose(InputContext::content_purpose_normal);
    m_engine->setSensitive(true);
    m_engine->update(QStringLiteral("teh"), QString());
    QCOMPARE(m_engine->autocorrection(), QString());
}

void CorrectionTest::autocorrectionOnUpdate()
{
    m_engine->update(QStringLiteral("I think teh"), QString());
    QCOMPARE(m_engine->currentWord(), QStringLiteral("teh"));
    QCOMPARE(m_engine->autocorrection(), QStringLiteral("the"));
    QCOMPARE(m_engine->suggestions().size(), 3);
    QCOMPARE(m_engine->suggestions().at(0), QStringLiteral("teh"));
    QCOMPARE(m_engine->suggestions().at(1), QStringLiteral("the"));
    m_engine->update(QStringLiteral("I think teh"), QStringLiteral("re"));
    QCOMPARE(m_engine->currentWord(), QStringLiteral("tehre"));
    QCOMPARE(m_engine->autocorrection(), QString());
}

void CorrectionTest::revertRemembersWord()
{
    m_engine->update(QStringLiteral("say teh"), QString());
    QCOMPARE(m_engine->autocorrection(), QStringLiteral("the"));
    m_engine->acceptWord(QStringLiteral("the"));
    m_engine->update(QStringLiteral("say the "), QString());
    QCOMPARE(m_engine->lastCorrectionOriginal(), QStringLiteral("teh"));
    QCOMPARE(m_engine->revertCorrection(), QStringLiteral("teh"));
    QCOMPARE(m_engine->revertCorrection(), QString());
    QVERIFY(m_engine->isLearned(QStringLiteral("teh")));
    m_engine->update(QStringLiteral("say teh"), QString());
    QCOMPARE(m_engine->autocorrection(), QString());
}

void CorrectionTest::expansions()
{
    QCOMPARE(m_engine->expansionFor(QStringLiteral("omw")), QStringLiteral("On my way!"));
    QCOMPARE(m_engine->expansionFor(QStringLiteral("nothing")), QString());
    m_engine->update(QStringLiteral("omw"), QString());
    QCOMPARE(m_engine->autocorrection(), QString());
    QCOMPARE(m_engine->suggestions().value(1), QStringLiteral("On my way!"));
}

QTEST_GUILESS_MAIN(CorrectionTest)
#include "correctiontest.moc"
