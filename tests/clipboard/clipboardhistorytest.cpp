#include "clipboardhistory.h"
#include "clipboarddetect.h"
#include "clipboardroles.h"
#include "kboardsettings.h"

#include <QFile>
#include <QMimeData>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTest>
#include <QTimeZone>

#include <memory>

namespace
{
constexpr qint64 s_minute = 60;
}

class ClipboardHistoryTest : public QObject
{
    Q_OBJECT

private:
    std::unique_ptr<QTemporaryDir> m_dir;
    QDateTime m_now;

    std::unique_ptr<ClipboardHistory> makeHistory()
    {
        ClipboardHistory::Options options;
        options.dataDirectory = m_dir->filePath(QStringLiteral("data"));
        options.cacheDirectory = m_dir->filePath(QStringLiteral("cache"));
        options.clock = [this] { return m_now; };
        return std::make_unique<ClipboardHistory>(options);
    }

    static QImage sampleImage(const QColor &color, const QSize &size = QSize(640, 360))
    {
        QImage image(size, QImage::Format_ARGB32);
        image.fill(color);
        return image;
    }

private Q_SLOTS:
    void initTestCase() { QStandardPaths::setTestModeEnabled(true); }

    void init()
    {
        KBoardSettings::self()->setDefaults();
        m_dir = std::make_unique<QTemporaryDir>();
        QVERIFY(m_dir->isValid());
        m_now = QDateTime(QDate(2026, 9, 18), QTime(12, 0), QTimeZone::UTC);
    }

    void cleanup() { m_dir.reset(); }

    void sensitiveExclusion()
    {
        auto history = makeHistory();

        QMimeData secret;
        secret.setText(QStringLiteral("hunter2"));
        secret.setData(QStringLiteral("x-kde-passwordManagerHint"), QByteArrayLiteral("secret"));
        QVERIFY(!history->addMimeData(&secret));
        QCOMPARE(history->count(), 0);

        history->mergeKlipperHistory({QStringLiteral("hunter2")}, false);
        QCOMPARE(history->count(), 0);

        bool sensitive = true;
        history->setSensitiveProbe([&sensitive] { return sensitive; });
        QVERIFY(!history->addText(QStringLiteral("typed into a password field")));
        QVERIFY(!history->addImage(sampleImage(Qt::red)));
        QCOMPARE(history->count(), 0);
        history->mergeKlipperHistory({QStringLiteral("typed into a password field")}, false);
        QCOMPARE(history->count(), 0);

        sensitive = false;
        QMimeData normal;
        normal.setText(QStringLiteral("hello"));
        normal.setData(QStringLiteral("x-kde-passwordManagerHint"), QByteArrayLiteral("public"));
        QVERIFY(history->addMimeData(&normal));
        QCOMPARE(history->count(), 1);
        QCOMPARE(history->data(history->index(0), ClipboardRoles::TextRole).toString(), QStringLiteral("hello"));
    }

    void otpDetection_data()
    {
        QTest::addColumn<QString>("text");
        QTest::addColumn<QString>("code");
        QTest::newRow("bare six") << QStringLiteral("482913") << QStringLiteral("482913");
        QTest::newRow("bare four") << QStringLiteral(" 4821 ") << QStringLiteral("4821");
        QTest::newRow("split") << QStringLiteral("123-456") << QStringLiteral("123456");
        QTest::newRow("sms") << QStringLiteral("Your verification code is 739201. Do not share it.") << QStringLiteral("739201");
        QTest::newRow("prefix") << QStringLiteral("G-551204 is your Google verification code.") << QStringLiteral("551204");
        QTest::newRow("pin") << QStringLiteral("Your PIN: 8812") << QStringLiteral("8812");
        QTest::newRow("eight") << QStringLiteral("Login code 12345678") << QStringLiteral("12345678");
        QTest::newRow("no keyword") << QStringLiteral("Meet me at room 4521 tomorrow") << QString();
        QTest::newRow("date") << QStringLiteral("Your code expires 2026-09-18") << QString();
        QTest::newRow("price") << QStringLiteral("Confirm payment of $1250") << QString();
        QTest::newRow("time") << QStringLiteral("Login at 12:30") << QString();
        QTest::newRow("too long") << QStringLiteral("123456789") << QString();
        QTest::newRow("too short") << QStringLiteral("123") << QString();
        QTest::newRow("phone") << QStringLiteral("Call +15551234567 for your code") << QString();
        QTest::newRow("prose") << QStringLiteral("Just some regular text") << QString();
    }

    void otpDetection()
    {
        QFETCH(QString, text);
        QFETCH(QString, code);
        QCOMPARE(ClipboardDetect::otpCode(text), code);

        auto history = makeHistory();
        QVERIFY(history->addText(text));
        QCOMPARE(history->data(history->index(0), ClipboardRoles::IsOtpRole).toBool(), !code.isEmpty());
        QCOMPARE(history->data(history->index(0), ClipboardRoles::OtpCodeRole).toString(), code);
        QCOMPARE(history->data(history->index(0), ClipboardRoles::KindRole).toString(),
            code.isEmpty() ? QStringLiteral("text") : QStringLiteral("otp"));
    }

    void urlDetection_data()
    {
        QTest::addColumn<QString>("text");
        QTest::addColumn<bool>("isUrl");
        QTest::addColumn<QString>("domain");
        QTest::newRow("https") << QStringLiteral("https://www.kde.org/plasma-desktop/") << true << QStringLiteral("kde.org");
        QTest::newRow("http query") << QStringLiteral(" http://invent.kde.org/plasma/kwin?x=1#y ") << true
                                    << QStringLiteral("invent.kde.org");
        QTest::newRow("www") << QStringLiteral("www.archlinux.org/packages") << true << QStringLiteral("archlinux.org");
        QTest::newRow("sentence") << QStringLiteral("see https://kde.org for more") << false << QString();
        QTest::newRow("file") << QStringLiteral("/home/user/file.txt") << false << QString();
        QTest::newRow("scheme only") << QStringLiteral("https://") << false << QString();
        QTest::newRow("word") << QStringLiteral("example") << false << QString();
    }

    void urlDetection()
    {
        QFETCH(QString, text);
        QFETCH(bool, isUrl);
        QFETCH(QString, domain);
        QCOMPARE(ClipboardDetect::isWebUrl(text), isUrl);

        auto history = makeHistory();
        QVERIFY(history->addText(text));
        QCOMPARE(history->data(history->index(0), ClipboardRoles::IsUrlRole).toBool(), isUrl);
        QCOMPARE(history->data(history->index(0), ClipboardRoles::DomainRole).toString(), domain);
    }

    void dedupe()
    {
        auto history = makeHistory();
        QVERIFY(history->addText(QStringLiteral("alpha")));
        QVERIFY(history->addText(QStringLiteral("beta")));
        m_now = m_now.addSecs(5);
        QSignalSpy moved(history.get(), &QAbstractItemModel::rowsMoved);
        QVERIFY(history->addText(QStringLiteral("alpha")));
        QCOMPARE(history->count(), 2);
        QCOMPARE(moved.count(), 1);
        QCOMPARE(history->data(history->index(0), ClipboardRoles::TextRole).toString(), QStringLiteral("alpha"));
        QCOMPARE(history->data(history->index(0), ClipboardRoles::TimestampRole).toDateTime(), m_now);

        QVERIFY(history->addImage(sampleImage(Qt::blue)));
        QVERIFY(history->addImage(sampleImage(Qt::green)));
        QVERIFY(history->addImage(sampleImage(Qt::blue)));
        QCOMPARE(history->count(), 4);
        QCOMPARE(history->data(history->index(0), ClipboardRoles::KindRole).toString(), QStringLiteral("image"));

        history->mergeKlipperHistory({QStringLiteral("gamma"), QStringLiteral("beta"), QStringLiteral("alpha")}, false);
        QCOMPARE(history->count(), 5);

        const QString big = QString(5000, QLatin1Char('x'));
        QVERIFY(history->addText(big));
        QCOMPARE(history->data(history->index(0), ClipboardRoles::FullTextRole).toString(), big);
        QVERIFY(history->data(history->index(0), ClipboardRoles::TextRole).toString().size() < 1100);
    }

    void expiryMarksThenRemoves()
    {
        KBoardSettings::setClipboardExpiryMinutes(10);
        auto history = makeHistory();
        const QDateTime start = m_now;
        QVERIFY(!history->expiry().isActive());
        QVERIFY(history->addText(QStringLiteral("temporary")));
        QVERIFY(history->addText(QStringLiteral("keeper")));
        history->pin(0, true);
        QVERIFY(history->expiry().isActive());
        QCOMPARE(history->expiry().nextEventTime(), start.addSecs(9 * s_minute));
        QCOMPARE(qint64(history->expiry().interval()), 9 * s_minute * 1000);

        m_now = start.addSecs(9 * s_minute + 1);
        history->expireDue();
        QCOMPARE(history->count(), 2);
        QVERIFY(history->data(history->index(1), ClipboardRoles::ExpiringRole).toBool());
        QCOMPARE(history->expiry().nextEventTime(), start.addSecs(10 * s_minute));

        m_now = start.addSecs(10 * s_minute + 1);
        QSignalSpy removed(history.get(), &QAbstractItemModel::rowsRemoved);
        history->expireDue();
        QCOMPARE(removed.count(), 1);
        QCOMPARE(history->count(), 1);
        QVERIFY(history->entryAt(0).pinned);
        QVERIFY(!history->expiry().isActive());
        QVERIFY(!history->expiry().nextEventTime().isValid());

        history->mergeKlipperHistory({QStringLiteral("temporary")}, false);
        QCOMPARE(history->count(), 1);
    }

    void expiryTimerFiresWhenOverdue()
    {
        KBoardSettings::setClipboardExpiryMinutes(10);
        auto history = makeHistory();
        QVERIFY(history->addText(QStringLiteral("overdue")));
        m_now = m_now.addSecs(60 * s_minute);
        QVERIFY(history->addText(QStringLiteral("fresh copy")));
        QCOMPARE(history->count(), 2);
        QCOMPARE(history->expiry().interval(), 0);
        QSignalSpy removed(history.get(), &QAbstractItemModel::rowsRemoved);
        QVERIFY(removed.wait(1000));
        QCOMPARE(history->count(), 1);
        QCOMPARE(history->entryAt(0).text, QStringLiteral("fresh copy"));
    }

    void unpinRestartsClockAndZeroDisables()
    {
        KBoardSettings::setClipboardExpiryMinutes(10);
        auto history = makeHistory();
        QVERIFY(history->addText(QStringLiteral("pinned")));
        history->pin(0, true);
        QVERIFY(!history->expiry().isActive());
        m_now = m_now.addSecs(120 * s_minute);
        history->pin(0, false);
        QCOMPARE(history->entryAt(0).timestamp, m_now);
        QCOMPARE(history->expiry().nextEventTime(), m_now.addSecs(9 * s_minute));

        KBoardSettings::setClipboardExpiryMinutes(0);
        Q_EMIT KBoardSettings::self()->clipboardExpiryMinutesChanged();
        QVERIFY(!history->expiry().isActive());
    }

    void maxItems()
    {
        KBoardSettings::setClipboardMaxItems(5);
        auto history = makeHistory();
        QVERIFY(history->addText(QStringLiteral("pinned oldest")));
        history->pin(0, true);
        for (int i = 0; i < 10; ++i) {
            QVERIFY(history->addText(QStringLiteral("item %1").arg(i)));
        }
        QCOMPARE(history->count(), 5);
        QCOMPARE(history->pinnedCount(), 1);
        QCOMPARE(history->entryAt(0).text, QStringLiteral("item 9"));
        QCOMPARE(history->entryAt(4).text, QStringLiteral("pinned oldest"));
        QCOMPARE(history->entryAt(3).text, QStringLiteral("item 6"));
    }

    void pausedAndDisabled()
    {
        auto history = makeHistory();
        QSignalSpy pausedSpy(history.get(), &ClipboardHistory::pausedChanged);
        history->setPaused(true);
        QCOMPARE(pausedSpy.count(), 1);
        QVERIFY(!history->addText(QStringLiteral("while paused")));
        history->setPaused(false);
        KBoardSettings::setClipboardEnabled(false);
        QVERIFY(!history->addText(QStringLiteral("while disabled")));
        KBoardSettings::setClipboardEnabled(true);
        QVERIFY(history->addText(QStringLiteral("recording again")));
        QCOMPARE(history->count(), 1);
    }

    void latestChip()
    {
        auto history = makeHistory();
        QVERIFY(!history->latestFresh());
        QSignalSpy freshSpy(history.get(), &ClipboardHistory::latestFreshChanged);
        history->mergeKlipperHistory({QStringLiteral("old klipper item")}, false);
        QVERIFY(!history->latestFresh());
        QVERIFY(history->addText(QStringLiteral("Your code is 551122")));
        QVERIFY(history->latestFresh());
        QCOMPARE(history->latest().value(QStringLiteral("otpCode")).toString(), QStringLiteral("551122"));
        QCOMPARE(history->latest().value(QStringLiteral("kind")).toString(), QStringLiteral("otp"));
        history->remove(0);
        QVERIFY(!history->latestFresh());
        QVERIFY(history->latest().isEmpty());
        QCOMPARE(freshSpy.count(), 2);
    }

    void pinPersistence()
    {
        QString pinnedImageId;
        {
            auto history = makeHistory();
            QVERIFY(history->addText(QStringLiteral("forget me")));
            QVERIFY(history->addText(QStringLiteral("remember me")));
            history->pin(0, true);
            QVERIFY(history->addImage(sampleImage(Qt::magenta)));
            pinnedImageId = history->entryAt(0).id;
            history->pin(0, true);
            QVERIFY(history->addImage(sampleImage(Qt::yellow)));
            history->saveNow();
            history->waitForPendingWrites();
            QVERIFY(QFile::exists(history->images().directory() + QLatin1Char('/') + pinnedImageId + QStringLiteral(".png")));
            QCOMPARE(QFile(history->historyFile()).permissions() & (QFileDevice::ReadGroup | QFileDevice::ReadOther),
                QFileDevice::Permissions());
        }
        auto reloaded = makeHistory();
        QCOMPARE(reloaded->count(), 2);
        QCOMPARE(reloaded->pinnedCount(), 2);
        QCOMPARE(reloaded->rowForId(pinnedImageId), 0);
        QCOMPARE(reloaded->entryAt(1).text, QStringLiteral("remember me"));
        QVERIFY(!reloaded->images().image(pinnedImageId, true).isNull());
        QCOMPARE(reloaded->images().image(pinnedImageId, false).size(), QSize(640, 360));
        QCOMPARE(QDir(reloaded->images().directory()).entryList({QStringLiteral("*.png")}, QDir::Files).size(), 2);

        reloaded->clear();
        QCOMPARE(reloaded->count(), 2);
        reloaded->clear(true);
        QCOMPARE(reloaded->count(), 0);
        reloaded->saveNow();
        reloaded->waitForPendingWrites();
        QCOMPARE(QDir(reloaded->images().directory()).entryList({QStringLiteral("*.png")}, QDir::Files).size(), 0);
    }

    void jsonRoundTrip()
    {
        KBoardSettings::setClipboardPersist(true);
        QList<ClipboardEntry> saved;
        {
            auto history = makeHistory();
            QVERIFY(history->addText(QStringLiteral("plain text\nwith two lines ✓")));
            m_now = m_now.addSecs(1);
            QVERIFY(history->addText(QStringLiteral("https://kde.org/announcements")));
            m_now = m_now.addSecs(1);
            QVERIFY(history->addText(QStringLiteral("Your login code: 420 911")));
            m_now = m_now.addSecs(1);
            QVERIFY(history->addImage(sampleImage(Qt::cyan, QSize(1200, 800)), QStringLiteral("image/jpeg")));
            history->pin(2, true);
            for (int row = 0; row < history->count(); ++row) {
                saved.append(history->entryAt(row));
            }
            history->saveNow();
            history->waitForPendingWrites();
        }
        auto reloaded = makeHistory();
        QCOMPARE(reloaded->count(), saved.size());
        for (int row = 0; row < saved.size(); ++row) {
            const auto &before = saved.at(row);
            const auto &after = reloaded->entryAt(row);
            QCOMPARE(after.id, before.id);
            QCOMPARE(after.text, before.text);
            QCOMPARE(after.mimeType, before.mimeType);
            QCOMPARE(after.image, before.image);
            QCOMPARE(after.imageHash, before.imageHash);
            QCOMPARE(after.imageSize, before.imageSize);
            QCOMPARE(after.pinned, before.pinned);
            QCOMPARE(after.timestamp, before.timestamp);
            QCOMPARE(after.isUrl, before.isUrl);
            QCOMPARE(after.domain, before.domain);
            QCOMPARE(after.otpCode, before.otpCode);
        }
        QCOMPARE(reloaded->data(reloaded->index(0), ClipboardRoles::ThumbnailSourceRole).toString(),
            QStringLiteral("image://kboardclipboard/%1/thumb").arg(saved.first().id));
        QCOMPARE(reloaded->images().image(saved.first().id, true).size(), QSize(480, 320));
        QCOMPARE(reloaded->entryAt(1).otpCode, QStringLiteral("420911"));

        QVERIFY(reloaded->addImage(sampleImage(Qt::cyan, QSize(1200, 800))));
        QCOMPARE(reloaded->count(), saved.size());
    }
};

QTEST_MAIN(ClipboardHistoryTest)

#include "clipboardhistorytest.moc"
