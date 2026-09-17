#include "inputcontext.h"
#include "kboardsettings.h"
#include "typingengine.h"
#include "typingtestsupport.h"

#include <QTest>

class ContextTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void init();
    void capitalization_data();
    void capitalization();
    void contentHints();
    void currentWordSpan();
    void languageAndErrors();

private:
    std::unique_ptr<TypingEngine> m_engine;
};

void ContextTest::initTestCase()
{
    TypingTest::prepareEnvironment();
    m_engine = TypingTest::readyEngine();
    QVERIFY(m_engine->isReady());
    QVERIFY(m_engine->isGlideReady());
}

void ContextTest::init()
{
    TypingTest::resetSettings();
    m_engine->setContentHint(0);
    m_engine->setContentPurpose(InputContext::content_purpose_normal);
}

void ContextTest::capitalization_data()
{
    QTest::addColumn<QString>("before");
    QTest::addColumn<bool>("expected");
    QTest::newRow("empty") << QString() << true;
    QTest::newRow("sentence end") << QStringLiteral("Hello there. ") << true;
    QTest::newRow("question") << QStringLiteral("Really? ") << true;
    QTest::newRow("quoted end") << QStringLiteral("He said \"stop.\" ") << true;
    QTest::newRow("mid sentence") << QStringLiteral("Hello ") << false;
    QTest::newRow("inside word") << QStringLiteral("Hel") << false;
    QTest::newRow("abbreviation") << QStringLiteral("Ask Mr. ") << false;
    QTest::newRow("latin abbreviation") << QStringLiteral("fruit, e.g. ") << false;
    QTest::newRow("newline") << QStringLiteral("first line\n") << true;
    QTest::newRow("comma") << QStringLiteral("Well, ") << false;
    QTest::newRow("opening bracket") << QStringLiteral("Done. (") << true;
}

void ContextTest::capitalization()
{
    QFETCH(QString, before);
    QFETCH(bool, expected);
    m_engine->update(before, QString());
    QCOMPARE(m_engine->shouldCapitalize(), expected);
}

void ContextTest::contentHints()
{
    m_engine->update(QStringLiteral("Hello "), QString());
    m_engine->setContentHint(InputContext::content_hint_uppercase);
    QVERIFY(m_engine->shouldCapitalize());
    m_engine->setContentHint(InputContext::content_hint_titlecase);
    QVERIFY(m_engine->shouldCapitalize());
    m_engine->update(QString(), QString());
    m_engine->setContentHint(InputContext::content_hint_lowercase);
    QVERIFY(!m_engine->shouldCapitalize());
    m_engine->setContentHint(0);
    m_engine->setContentPurpose(InputContext::content_purpose_terminal);
    QVERIFY(!m_engine->shouldCapitalize());
    m_engine->setContentPurpose(InputContext::content_purpose_normal);
    KBoardSettings::self()->setAutoCapitalize(false);
    QVERIFY(!m_engine->shouldCapitalize());
}

void ContextTest::currentWordSpan()
{
    m_engine->update(QStringLiteral("I like tom"), QStringLiteral("atoes a lot"));
    QCOMPARE(m_engine->currentWord(), QStringLiteral("tomatoes"));
    QCOMPARE(m_engine->wordCharsBefore(), 3);
    QCOMPARE(m_engine->wordCharsAfter(), 5);
    m_engine->update(QStringLiteral("(don't"), QString());
    QCOMPARE(m_engine->currentWord(), QStringLiteral("don't"));
    m_engine->update(QStringLiteral("done "), QString());
    QCOMPARE(m_engine->currentWord(), QString());
}

void ContextTest::languageAndErrors()
{
    QCOMPARE(m_engine->language(), QStringLiteral("en"));
    QVERIFY(m_engine->errorString().isEmpty());
    QVERIFY(m_engine->isValidWord(QStringLiteral("hello")));
    QVERIFY(!m_engine->isValidWord(QStringLiteral("helo")));
    KBoardSettings::self()->setActiveLayout(QStringLiteral("unknownlayout"));
    QVERIFY(!m_engine->isReady());
    QVERIFY(!m_engine->errorString().isEmpty());
    KBoardSettings::self()->setActiveLayout(QStringLiteral("us"));
    QVERIFY(QTest::qWaitFor([this] { return m_engine->isReady(); }, 20000));
    QVERIFY(m_engine->errorString().isEmpty());
}

QTEST_GUILESS_MAIN(ContextTest)
#include "contexttest.moc"
