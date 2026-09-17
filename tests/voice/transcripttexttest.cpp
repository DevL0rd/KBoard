#include "transcripttext.h"

#include <QTest>

using TranscriptText::Action;

class TranscriptTextTest : public QObject
{
    Q_OBJECT

private:
    static Action insert(const QString &text) { return Action {Action::Insert, text}; }

    static Action newLine() { return Action {Action::NewLine, QString()}; }

    static QString describe(const QList<Action> &actions)
    {
        QStringList parts;
        for (const Action &action : actions) {
            switch (action.kind) {
            case Action::Insert:
                parts.append(QStringLiteral("insert[%1]").arg(action.text));
                break;
            case Action::NewLine:
                parts.append(QStringLiteral("newline"));
                break;
            case Action::DeletePrevious:
                parts.append(QStringLiteral("delete"));
                break;
            }
        }
        return parts.join(u' ');
    }

private Q_SLOTS:
    void commands_data()
    {
        QTest::addColumn<QString>("transcript");
        QTest::addColumn<QString>("expected");
        QTest::newRow("plain") << QStringLiteral("Hello there.") << QStringLiteral("insert[Hello there.]");
        QTest::newRow("new line") << QStringLiteral("Dear Sam, new line. Thanks for the call.")
                                  << QStringLiteral("insert[Dear Sam,] newline insert[Thanks for the call.]");
        QTest::newRow("newline word") << QStringLiteral("One newline two") << QStringLiteral("insert[One] newline insert[two]");
        QTest::newRow("new paragraph") << QStringLiteral("End. New paragraph. Start")
                                       << QStringLiteral("insert[End.] newline newline insert[Start]");
        QTest::newRow("period replaces model punctuation")
            << QStringLiteral("I am here, period. see you") << QStringLiteral("insert[I am here. See you]");
        QTest::newRow("full stop") << QStringLiteral("That is all full stop") << QStringLiteral("insert[That is all.]");
        QTest::newRow("comma") << QStringLiteral("Apples comma. Pears") << QStringLiteral("insert[Apples, pears]");
        QTest::newRow("question mark") << QStringLiteral("Are you coming question mark") << QStringLiteral("insert[Are you coming?]");
        QTest::newRow("exclamation") << QStringLiteral("Great exclamation mark. yes exclamation point")
                                     << QStringLiteral("insert[Great! Yes!]");
        QTest::newRow("leading punctuation") << QStringLiteral("Period.") << QStringLiteral("insert[.]");
        QTest::newRow("delete within utterance")
            << QStringLiteral("Wrong words delete that right words") << QStringLiteral("insert[right words]");
        QTest::newRow("delete previous utterance") << QStringLiteral("Delete that.") << QStringLiteral("delete");
        QTest::newRow("delete after newline") << QStringLiteral("Hi new line delete that") << QStringLiteral("insert[Hi]");
        QTest::newRow("double delete") << QStringLiteral("Delete that. Delete that.") << QStringLiteral("delete delete");
        QTest::newRow("empty") << QStringLiteral("   ") << QString();
    }

    void commands()
    {
        QFETCH(QString, transcript);
        QFETCH(QString, expected);
        QCOMPARE(describe(TranscriptText::applyCommands(transcript)), expected);
    }

    void plainKeepsCommandWords()
    {
        const QList<Action> expected {insert(QStringLiteral("Say new line period"))};
        QCOMPARE(TranscriptText::plain(QStringLiteral(" Say  new line period ")), expected);
        QVERIFY(TranscriptText::plain(QString()).isEmpty());
        QCOMPARE(TranscriptText::applyCommands(QStringLiteral("a new line")).last(), newLine());
    }

    void fitToContext_data()
    {
        QTest::addColumn<QString>("text");
        QTest::addColumn<QString>("before");
        QTest::addColumn<QString>("expected");
        QTest::newRow("empty field") << QStringLiteral("hello world.") << QString() << QStringLiteral("Hello world.");
        QTest::newRow("mid sentence") << QStringLiteral("Going home.") << QStringLiteral("I am") << QStringLiteral(" going home.");
        QTest::newRow("after space") << QStringLiteral("Going home.") << QStringLiteral("I am ") << QStringLiteral("going home.");
        QTest::newRow("after period") << QStringLiteral("then we left.") << QStringLiteral("It rained.")
                                      << QStringLiteral(" Then we left.");
        QTest::newRow("after question and space") << QStringLiteral("yes.") << QStringLiteral("Why? ") << QStringLiteral("Yes.");
        QTest::newRow("after newline") << QStringLiteral("next") << QStringLiteral("Line\n") << QStringLiteral("Next");
        QTest::newRow("pronoun I") << QStringLiteral("I think so.") << QStringLiteral("and") << QStringLiteral(" I think so.");
        QTest::newRow("contraction I'm") << QStringLiteral("I'm late.") << QStringLiteral("so") << QStringLiteral(" I'm late.");
        QTest::newRow("acronym") << QStringLiteral("NASA called.") << QStringLiteral("then") << QStringLiteral(" NASA called.");
        QTest::newRow("punctuation attaches") << QStringLiteral(",") << QStringLiteral("apples") << QStringLiteral(",");
        QTest::newRow("after bracket") << QStringLiteral("Maybe") << QStringLiteral("see (") << QStringLiteral("maybe");
        QTest::newRow("whitespace only") << QStringLiteral("  ") << QStringLiteral("x") << QString();
    }

    void fitToContext()
    {
        QFETCH(QString, text);
        QFETCH(QString, before);
        QFETCH(QString, expected);
        QCOMPARE(TranscriptText::fitToContext(text, before), expected);
    }
};

QTEST_GUILESS_MAIN(TranscriptTextTest)
#include "transcripttexttest.moc"
