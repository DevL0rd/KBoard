#include "keygestures.h"

#include <QSignalSpy>
#include <QTest>

class TestKeyGestures : public QObject
{
    Q_OBJECT

private:
    struct Fixture
    {
        QList<QVariantMap> keys;
        std::unique_ptr<KeyGestures> gestures;
    };

    static QVariantMap key(const QString &type, bool glide = false, bool repeat = false)
    {
        return {{QStringLiteral("type"), type}, {QStringLiteral("glide"), glide}, {QStringLiteral("repeat"), repeat}};
    }

    static std::unique_ptr<Fixture> fixture()
    {
        auto f = std::make_unique<Fixture>();
        f->keys = {key(QStringLiteral("char"), true), key(QStringLiteral("char"), true), key(QStringLiteral("char"), true),
            key(QStringLiteral("space")), key(QStringLiteral("backspace"), false, true), key(QStringLiteral("shift"))};
        Fixture *raw = f.get();
        KeyGestures::Keys keys {
            [raw](QPointF point) {
                const int index = int(point.x() / 100);
                return index >= 0 && index < raw->keys.size() ? index : -1;
            },
            [raw](int index) { return index >= 0 && index < raw->keys.size() ? raw->keys.at(index) : QVariantMap(); },
            [] { return 100.0; },
        };
        f->gestures = std::make_unique<KeyGestures>(std::move(keys));
        f->gestures->setProperty("longPressDelay", 60);
        f->gestures->setProperty("repeatDelay", 60);
        f->gestures->setProperty("repeatInterval", 20);
        return f;
    }

private Q_SLOTS:
    void tapReleasesWithTapMode()
    {
        auto f = fixture();
        QSignalSpy released(f->gestures.get(), &KeyGestures::keyReleased);
        QVERIFY(f->gestures->press(1, QPointF(50, 10)));
        QCOMPARE(f->gestures->pressedIndexes(), QList<int> {0});
        f->gestures->release(1, QPointF(52, 12));
        QCOMPARE(released.size(), 1);
        QCOMPARE(released.at(0).at(4).toString(), QStringLiteral("tap"));
        QVERIFY(f->gestures->pressedIndexes().isEmpty());
    }

    void secondFingerCommitsTheFirstKeyInOrder()
    {
        auto f = fixture();
        QSignalSpy released(f->gestures.get(), &KeyGestures::keyReleased);
        f->gestures->press(1, QPointF(50, 10));
        f->gestures->press(2, QPointF(150, 10));
        QCOMPARE(released.size(), 1);
        QCOMPARE(released.at(0).at(1).toInt(), 0);
        f->gestures->release(2, QPointF(150, 10));
        f->gestures->release(1, QPointF(50, 10));
        QCOMPARE(released.size(), 3);
        QCOMPARE(released.at(1).at(4).toString(), QStringLiteral("tap"));
        QCOMPARE(released.at(2).at(4).toString(), QStringLiteral("done"));
    }

    void holdingEmitsLongPress()
    {
        auto f = fixture();
        QSignalSpy longPressed(f->gestures.get(), &KeyGestures::keyLongPressed);
        QSignalSpy released(f->gestures.get(), &KeyGestures::keyReleased);
        f->gestures->press(1, QPointF(250, 10));
        QVERIFY(longPressed.wait(500));
        f->gestures->setTouchMode(1, QStringLiteral("popup"));
        f->gestures->release(1, QPointF(250, 10));
        QCOMPARE(released.constLast().at(4).toString(), QStringLiteral("popup"));
    }

    void backspaceRepeatsFasterOverTime()
    {
        auto f = fixture();
        QSignalSpy repeated(f->gestures.get(), &KeyGestures::keyRepeated);
        QSignalSpy released(f->gestures.get(), &KeyGestures::keyReleased);
        f->gestures->press(1, QPointF(450, 10));
        QTRY_VERIFY_WITH_TIMEOUT(repeated.size() >= 5, 2000);
        f->gestures->release(1, QPointF(450, 10));
        QCOMPARE(released.constLast().at(4).toString(), QStringLiteral("repeat"));
    }

    void glideCollectsPointsAcrossLetters()
    {
        auto f = fixture();
        QSignalSpy started(f->gestures.get(), &KeyGestures::glideStarted);
        QSignalSpy finished(f->gestures.get(), &KeyGestures::glideFinished);
        QSignalSpy released(f->gestures.get(), &KeyGestures::keyReleased);
        f->gestures->press(1, QPointF(50, 10));
        f->gestures->move(1, QPointF(120, 12));
        f->gestures->move(1, QPointF(180, 14));
        f->gestures->move(1, QPointF(250, 16));
        f->gestures->release(1, QPointF(260, 16));
        QCOMPARE(started.size(), 1);
        QCOMPARE(finished.size(), 1);
        QVERIFY(finished.at(0).at(0).toList().size() >= 4);
        QVERIFY(released.isEmpty());
    }

    void spaceSwipeMovesTheCursor()
    {
        auto f = fixture();
        QSignalSpy cursor(f->gestures.get(), &KeyGestures::cursorSwipe);
        QSignalSpy done(f->gestures.get(), &KeyGestures::cursorSwipeFinished);
        f->gestures->press(1, QPointF(350, 10));
        f->gestures->move(1, QPointF(260, 10));
        f->gestures->move(1, QPointF(200, 10));
        f->gestures->release(1, QPointF(200, 10));
        int steps = 0;
        for (const auto &call : cursor) {
            steps += call.at(0).toInt();
        }
        QCOMPARE(steps, -3);
        QCOMPARE(done.size(), 1);
    }

    void backspaceSwipeSelectsWords()
    {
        auto f = fixture();
        QSignalSpy words(f->gestures.get(), &KeyGestures::deleteSwipe);
        QSignalSpy finished(f->gestures.get(), &KeyGestures::deleteSwipeFinished);
        f->gestures->press(1, QPointF(450, 10));
        f->gestures->move(1, QPointF(380, 10));
        f->gestures->move(1, QPointF(300, 10));
        f->gestures->release(1, QPointF(300, 10));
        QCOMPARE(words.constLast().at(0).toInt(), 2);
        QCOMPARE(finished.at(0).at(0).toInt(), 2);
    }

    void nonGlideKeysSlideToTheNextKey()
    {
        auto f = fixture();
        f->gestures->setProperty("glideEnabled", false);
        QSignalSpy moved(f->gestures.get(), &KeyGestures::pressMoved);
        f->gestures->press(1, QPointF(50, 10));
        f->gestures->move(1, QPointF(150, 10));
        QCOMPARE(moved.size(), 1);
        QCOMPARE(f->gestures->pressedIndexes(), QList<int> {1});
    }

    void cancelAllReportsCancel()
    {
        auto f = fixture();
        QSignalSpy released(f->gestures.get(), &KeyGestures::keyReleased);
        f->gestures->press(1, QPointF(550, 10));
        f->gestures->cancelAll();
        QCOMPARE(released.at(0).at(4).toString(), QStringLiteral("cancel"));
        QCOMPARE(f->gestures->activeTouches(), 0);
    }
};

QTEST_GUILESS_MAIN(TestKeyGestures)
#include "testkeygestures.moc"
