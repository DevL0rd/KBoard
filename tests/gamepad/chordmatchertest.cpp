#include "chordmatcher.h"

#include <QTest>

class ChordMatcherTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void parsing_data()
    {
        QTest::addColumn<QString>("chord");
        QTest::addColumn<QStringList>("buttons");
        QTest::addColumn<bool>("valid");
        QTest::newRow("default") << QStringLiteral("back+x") << QStringList {QStringLiteral("back"), QStringLiteral("x")} << true;
        QTest::newRow("aliases and case") << QStringLiteral(
            " Select + LB + rt ") << QStringList {QStringLiteral("back"), QStringLiteral("leftshoulder"), QStringLiteral("righttrigger")}
                                          << true;
        QTest::newRow("duplicates") << QStringLiteral("x+x+select+back") << QStringList {QStringLiteral("x"), QStringLiteral("back")}
                                    << true;
        QTest::newRow("single") << QStringLiteral("guide") << QStringList {QStringLiteral("guide")} << true;
        QTest::newRow("empty") << QString() << QStringList {} << false;
        QTest::newRow("only plus") << QStringLiteral("++") << QStringList {} << false;
        QTest::newRow("unknown") << QStringLiteral("back+banana") << QStringList {} << false;
    }

    void parsing()
    {
        QFETCH(QString, chord);
        QFETCH(QStringList, buttons);
        QFETCH(bool, valid);
        const ChordMatcher::Parsed parsed = ChordMatcher::parse(chord);
        QCOMPARE(parsed.buttons, buttons);
        QCOMPARE(parsed.error.isEmpty(), valid);
        ChordMatcher matcher;
        QCOMPARE(matcher.setChord(chord), valid);
        QCOMPARE(matcher.isValid(), valid);
    }

    void unknownButtonError()
    {
        ChordMatcher matcher;
        QVERIFY(!matcher.setChord(QStringLiteral("back+banana")));
        QVERIFY(matcher.errorString().contains(QStringLiteral("banana")));
        QVERIFY(!matcher.press(QStringLiteral("back")));
    }

    void firesOncePerPress()
    {
        ChordMatcher matcher;
        QVERIFY(matcher.setChord(QStringLiteral("back+x")));
        QVERIFY(!matcher.press(QStringLiteral("back")));
        QVERIFY(matcher.press(QStringLiteral("x")));
        QVERIFY(!matcher.press(QStringLiteral("x")));
        QVERIFY(!matcher.press(QStringLiteral("a")));
        matcher.release(QStringLiteral("a"));
        QVERIFY(!matcher.press(QStringLiteral("x")));
        matcher.release(QStringLiteral("x"));
        QVERIFY(matcher.press(QStringLiteral("x")));
        matcher.release(QStringLiteral("x"));
        matcher.release(QStringLiteral("back"));
        QVERIFY(!matcher.press(QStringLiteral("x")));
        QVERIFY(matcher.press(QStringLiteral("back")));
    }

    void orderIndependent()
    {
        ChordMatcher matcher;
        QVERIFY(matcher.setChord(QStringLiteral("back+x+lb")));
        QVERIFY(!matcher.press(QStringLiteral("leftshoulder")));
        QVERIFY(!matcher.press(QStringLiteral("x")));
        QVERIFY(!matcher.press(QStringLiteral("y")));
        QVERIFY(matcher.press(QStringLiteral("back")));
    }

    void extraButtonsDoNotBlock()
    {
        ChordMatcher matcher;
        QVERIFY(matcher.setChord(QStringLiteral("back+x")));
        QVERIFY(!matcher.press(QStringLiteral("a")));
        QVERIFY(!matcher.press(QStringLiteral("back")));
        QVERIFY(matcher.press(QStringLiteral("x")));
    }

    void resetClearsHeld()
    {
        ChordMatcher matcher;
        QVERIFY(matcher.setChord(QStringLiteral("back+x")));
        QVERIFY(!matcher.press(QStringLiteral("back")));
        matcher.reset();
        QVERIFY(!matcher.press(QStringLiteral("x")));
    }
};

QTEST_GUILESS_MAIN(ChordMatcherTest)
#include "chordmatchertest.moc"
