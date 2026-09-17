#include "emojigridmodel.h"
#include "emojilistmodel.h"
#include "emojistore.h"

#include "kboardpaths.h"
#include "kboardsettings.h"

#include <QDir>
#include <QElapsedTimer>
#include <QStandardPaths>
#include <QTest>

using namespace Qt::Literals::StringLiterals;

class EmojiTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        qputenv("KBOARD_USE_BUILD_TREE", "1");
        QStandardPaths::setTestModeEnabled(true);
        QDir(KBoardPaths::userDataFile(u"emoji"_s)).removeRecursively();
        KBoardSettings::setSkinTone(0);
    }

    void cleanupTestCase() { QDir(KBoardPaths::userDataFile(u"emoji"_s)).removeRecursively(); }

    void dataLoads()
    {
        QElapsedTimer timer;
        timer.start();
        EmojiStore store;
        qInfo("EmojiStore loaded in %lld ms", timer.elapsed());
        QVERIFY2(store.isReady(), qPrintable(store.errorString()));
        QVERIFY(store.errorString().isEmpty());
        QVERIFY(store.count() > 3500);
        QVERIFY(store.data().entries().size() > 1800);
        QCOMPARE(store.emojiVersion(), u"18.0"_s);

        const QVariantList groups = store.groups();
        QCOMPARE(groups.size(), 11);
        QCOMPARE(groups.first().toMap().value(u"id"_s).toString(), u"recents"_s);
        QCOMPARE(groups.at(1).toMap().value(u"name"_s).toString(), u"Smileys & Emotion"_s);
        QCOMPARE(groups.at(9).toMap().value(u"id"_s).toString(), u"flags"_s);
        QCOMPARE(groups.last().toMap().value(u"id"_s).toString(), u"kaomoji"_s);
        QVERIFY(store.data().kaomoji().size() > 100);

        QAbstractItemModel *smileys = store.modelForGroup(u"smileys"_s);
        QVERIFY(smileys);
        QVERIFY(smileys->rowCount() > 100);
        QCOMPARE(smileys->index(0, 0).data(EmojiListModel::EmojiRole).toString(), u"😀"_s);
        QCOMPARE(smileys->index(0, 0).data(EmojiListModel::NameRole).toString(), u"grinning face"_s);
        QVERIFY(store.modelForGroup(u"flags"_s)->rowCount() > 250);
        int total = 0;
        for (const QString &id :
            {u"smileys"_s, u"people"_s, u"animals"_s, u"food"_s, u"travel"_s, u"activities"_s, u"objects"_s, u"symbols"_s, u"flags"_s}) {
            total += store.modelForGroup(id)->rowCount();
        }
        QVERIFY(total > 1800);
    }

    void search()
    {
        EmojiStore store;
        const QStringList heart = store.searchEmoji(u"heart"_s, 20);
        QVERIFY(heart.size() >= 10);
        QVERIFY(heart.mid(0, 3).contains(u"❤️"_s));

        const QStringList thumbs = store.searchEmoji(u"thumbs"_s, 10);
        QVERIFY(thumbs.size() >= 2);
        QVERIFY(thumbs.mid(0, 2).contains(u"👍"_s));
        QVERIFY(thumbs.mid(0, 2).contains(u"👎"_s));

        QCOMPARE(store.searchEmoji(u"thumbs up"_s, 5).value(0), u"👍"_s);
        QCOMPARE(store.searchEmoji(u"piz"_s, 5).value(0), u"🍕"_s);
        QVERIFY(store.searchEmoji(u"xyzzyqq"_s).isEmpty());
        QVERIFY(store.searchEmoji(u"   "_s).isEmpty());
        QVERIFY(store.searchKaomoji(u"shrug"_s).contains(u"¯\\_(ツ)_/¯"_s));

        QElapsedTimer timer;
        timer.start();
        for (int i = 0; i < 20; ++i) {
            store.searchEmoji(u"s"_s, 200);
        }
        QVERIFY2(timer.elapsed() < 400, qPrintable(u"search too slow: %1 ms"_s.arg(timer.elapsed())));
    }

    void skinTones()
    {
        EmojiStore store;
        QCOMPARE(store.withSkinTone(u"👍"_s, 0), u"👍"_s);
        QCOMPARE(store.withSkinTone(u"👍"_s, 1), u"👍🏻"_s);
        QCOMPARE(store.withSkinTone(u"👍"_s, 5), u"👍🏿"_s);
        QCOMPARE(store.withSkinTone(u"👍🏽"_s, 2), u"👍🏼"_s);
        QCOMPARE(store.withSkinTone(u"👍🏽"_s, 0), u"👍"_s);
        QCOMPARE(store.withSkinTone(u"🕵️"_s, 3), u"🕵🏽"_s);
        QCOMPARE(store.withSkinTone(u"🧑‍🤝‍🧑"_s, 4), u"🧑🏾‍🤝‍🧑🏾"_s);
        QCOMPARE(store.withSkinTone(u"👩‍❤️‍👨"_s, 1), u"👩🏻‍❤️‍👨🏻"_s);
        QCOMPARE(store.withSkinTone(u"🧑‍💻"_s, 2), u"🧑🏼‍💻"_s);
        QCOMPARE(store.withSkinTone(u"🍕"_s, 3), u"🍕"_s);
        QCOMPARE(store.withSkinTone(u"🤝"_s, 5), u"🤝🏿"_s);
        QVERIFY(store.hasSkinTones(u"👋"_s));
        QVERIFY(!store.hasSkinTones(u"😀"_s));
        QCOMPARE(store.baseEmoji(u"👋🏾"_s), u"👋"_s);
        QCOMPARE(store.skinToneOf(u"👋🏾"_s), 4);
        QCOMPARE(store.skinToneVariants(u"👋"_s).size(), 6);
        QCOMPARE(store.nameOf(u"👋🏾"_s), u"waving hand: medium-dark skin tone"_s);

        QCOMPARE(store.displayEmoji(u"👋"_s), u"👋"_s);
        KBoardSettings::setSkinTone(3);
        QCOMPARE(store.displayEmoji(u"👋"_s), u"👋🏽"_s);
        store.setPreferredTone(u"👋"_s, 1);
        QCOMPARE(store.displayEmoji(u"👋"_s), u"👋🏻"_s);
        QCOMPARE(store.displayEmoji(u"👍"_s), u"👍🏽"_s);
        store.recordUse(u"👍🏿"_s);
        QCOMPARE(store.preferredTone(u"👍"_s), 5);

        EmojiStore reloaded;
        QCOMPARE(reloaded.preferredTone(u"👋"_s), 1);
        QCOMPARE(reloaded.preferredTone(u"👍"_s), 5);
        KBoardSettings::setSkinTone(0);
        QCOMPARE(reloaded.displayEmoji(u"👌"_s), u"👌"_s);
    }

    void recentsPersist()
    {
        {
            EmojiStore store;
            store.clearRecents();
            store.recordUse(u"🍕"_s);
            store.recordUse(u"😂"_s);
            store.recordUse(u"🍕"_s);
            QCOMPARE(store.recentList(), (QStringList {u"🍕"_s, u"😂"_s}));
            QCOMPARE(store.recentsModel()->rowCount(), 2);
            QCOMPARE(store.usageCount(u"🍕"_s), 2);
            store.toggleFavorite(u"🔥"_s);
            store.toggleFavorite(u"👋🏽"_s);
            QVERIFY(store.isFavorite(u"👋"_s));
        }
        {
            EmojiStore store;
            QCOMPARE(store.recentList(), (QStringList {u"🍕"_s, u"😂"_s}));
            QCOMPARE(store.usageCount(u"🍕"_s), 2);
            QCOMPARE(store.favoriteList(), (QStringList {u"👋"_s, u"🔥"_s}));
            store.toggleFavorite(u"🔥"_s);
            QCOMPARE(store.favoriteList(), (QStringList {u"👋"_s}));

            const int limit = KBoardSettings::emojiRecentsLimit();
            const QStringList smileys = store.searchEmoji(u"face"_s, limit + 20);
            QVERIFY(smileys.size() > limit);
            for (const QString &emoji : smileys) {
                store.recordUse(emoji);
            }
            QCOMPARE(store.recentList().size(), limit);
            QCOMPARE(store.recentList().first(), smileys.last());
        }
        {
            EmojiStore store;
            QCOMPARE(store.recentList().size(), KBoardSettings::emojiRecentsLimit());
            store.clearRecents();
        }
        EmojiStore store;
        QVERIFY(store.recentList().isEmpty());
    }

    void suggestions()
    {
        EmojiStore store;
        QCOMPARE(store.suggestionsFor(u"pizza"_s).value(0), u"🍕"_s);
        QCOMPARE(store.suggestionsFor(u"Pizza!"_s).value(0), u"🍕"_s);
        const QStringList love = store.suggestionsFor(u"love"_s);
        QCOMPARE(love.size(), 3);
        QCOMPARE(love.first(), u"❤️"_s);
        qInfo("love -> %s", qPrintable(love.join(u' ')));
        QCOMPARE(love.at(1), u"😍"_s);
        QVERIFY(store.suggestionsFor(u"cats"_s).contains(u"🐱"_s) || store.suggestionsFor(u"cats"_s).contains(u"🐈"_s));
        QVERIFY(store.suggestionsFor(u"the"_s).isEmpty());
        QVERIFY(store.suggestionsFor(u"a"_s).isEmpty());
        QVERIFY(store.suggestionsFor(u"qwrtzp"_s).isEmpty());
        QCOMPARE(store.suggestionsFor(u"fire"_s).value(0), u"🔥"_s);
    }

    void gridModel()
    {
        EmojiGridModel model;
        model.setColumns(9);
        QVERIFY(model.rowCount() > 200);
        QCOMPARE(model.sectionAt(0), u"recents"_s);
        const int smileys = model.rowForSection(u"smileys"_s);
        QVERIFY(smileys > 0);
        QCOMPARE(model.index(smileys).data(EmojiGridModel::KindRole).toString(), u"header"_s);
        const QVariantList cells = model.index(smileys + 1).data(EmojiGridModel::CellsRole).toList();
        QCOMPARE(cells.size(), 9);
        QCOMPARE(cells.first().toMap().value(u"text"_s).toString(), u"😀"_s);
        QVERIFY(model.rowForSection(u"kaomoji"_s) > model.rowForSection(u"flags"_s));
        QCOMPARE(model.sections().size(), 11);

        model.setSearchText(u"heart"_s);
        QCOMPARE(model.sectionAt(0), u"results"_s);
        QCOMPARE(model.index(0).data(EmojiGridModel::KindRole).toString(), u"emoji"_s);
        model.setSearchText(u"zzzqqq"_s);
        QCOMPARE(model.rowCount(), 1);
        QCOMPARE(model.index(0).data(EmojiGridModel::KindRole).toString(), u"empty"_s);
    }
};

QTEST_MAIN(EmojiTest)

#include "emojitest.moc"
