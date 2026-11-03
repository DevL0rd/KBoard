#include "keylayout.h"
#include "layouts.h"

#include <QTest>

class TestKeyLayout : public QObject
{
    Q_OBJECT

private:
    Layouts m_layouts {QStringLiteral(KBOARD_LAYOUT_DIR)};

    QVariantMap letters() const { return m_layouts.page(QStringLiteral("us"), QStringLiteral("letters")); }

    static QVariantMap key(const QString &type, double width) { return {{QStringLiteral("type"), type}, {QStringLiteral("width"), width}}; }

    static int find(const QList<KeyEntry> &entries, const QString &label, int half = -2)
    {
        for (int i = 0; i < entries.size(); ++i) {
            if (entries.at(i).key.value(QStringLiteral("label")).toString() == label && (half == -2 || entries.at(i).half == half)) {
                return i;
            }
        }
        return -1;
    }

private Q_SLOTS:
    void splitPointPrefersTheLargerLeftHalfOnTies()
    {
        const QVariantList row {key(QStringLiteral("shift"), 1.5), key(QStringLiteral("char"), 1), key(QStringLiteral("char"), 1),
            key(QStringLiteral("char"), 1), key(QStringLiteral("char"), 1), key(QStringLiteral("char"), 1), key(QStringLiteral("char"), 1),
            key(QStringLiteral("char"), 1), key(QStringLiteral("backspace"), 1.5)};
        QCOMPARE(KeyLayout::splitPoint(row).index, 5);
        QCOMPARE(KeyLayout::splitPoint(row).leftPart, 0.0);
    }

    void splitPointCutsTheSpaceBar()
    {
        const QVariantList row {key(QStringLiteral("symbols"), 1.5), key(QStringLiteral("char"), 1), key(QStringLiteral("emoji"), 1),
            key(QStringLiteral("space"), 4), key(QStringLiteral("char"), 1), key(QStringLiteral("enter"), 1.5)};
        const KeyLayout::SplitPoint point = KeyLayout::splitPoint(row);
        QCOMPARE(point.index, 3);
        QCOMPARE(point.leftPart, 1.5);
    }

    void fullWidthFillsTheWholeArea()
    {
        const auto entries = KeyLayout::compute(letters(), {1000, 400, 10, false, 0});
        QCOMPARE(entries.size(), 10 + 9 + 9 + 6);
        const KeyEntry &q = entries.at(find(entries, QStringLiteral("q")));
        QCOMPARE(q.rect, QRectF(5, 5, 90, 90));
        QCOMPARE(q.cell.left(), 0.0);
        const KeyEntry &a = entries.at(find(entries, QStringLiteral("a")));
        QCOMPARE(a.rect.left(), 55.0);
        QCOMPARE(a.cell.left(), 0.0);
        QCOMPARE(KeyLayout::keyAt(entries, QPointF(2, 150)), find(entries, QStringLiteral("a")));
        QCOMPARE(KeyLayout::keyAt(entries, QPointF(999, 20)), find(entries, QStringLiteral("p")));
    }

    void splitModeLeavesAGapWithoutKeys()
    {
        const KeyLayout::Params params {2000, 400, 10, true, 800};
        const auto entries = KeyLayout::compute(letters(), params);
        QCOMPARE(KeyLayout::halfWidth(params), 600.0);
        QCOMPARE(KeyLayout::keyAt(entries, QPointF(1000, 50)), -1);
        QVERIFY(find(entries, QStringLiteral("t"), 0) >= 0);
        QVERIFY(find(entries, QStringLiteral("y"), 1) >= 0);
        QVERIFY(find(entries, QStringLiteral("v"), 0) >= 0);
        QVERIFY(find(entries, QStringLiteral("b"), 1) >= 0);
        QCOMPARE(find(entries, QStringLiteral(" "), 0) >= 0 && find(entries, QStringLiteral(" "), 1) >= 0, true);
        for (const KeyEntry &entry : entries) {
            QVERIFY(entry.rect.right() <= 600 || entry.rect.left() >= 1400);
        }
        QCOMPARE(KeyLayout::halfRects(params).size(), 2);
    }

    void neighborsMoveWithinRowsAndToNearestColumns()
    {
        const auto entries = KeyLayout::compute(letters(), {1000, 400, 10, false, 0});
        const int q = find(entries, QStringLiteral("q"));
        QCOMPARE(KeyLayout::neighbor(entries, q, 1, 0), find(entries, QStringLiteral("w")));
        QCOMPARE(KeyLayout::neighbor(entries, q, 0, 1), find(entries, QStringLiteral("a")));
        QCOMPARE(KeyLayout::neighbor(entries, find(entries, QStringLiteral("p")), 0, 1), find(entries, QStringLiteral("l")));
    }

    void navigationWrapsAroundTheEdges()
    {
        const auto entries = KeyLayout::compute(letters(), {1000, 400, 10, false, 0});
        const int q = find(entries, QStringLiteral("q"));
        const int p = find(entries, QStringLiteral("p"));
        QCOMPARE(KeyLayout::neighbor(entries, q, -1, 0), p);
        QCOMPARE(KeyLayout::neighbor(entries, p, 1, 0), q);

        const int lastRow = entries.last().row;
        const int up = KeyLayout::neighbor(entries, q, 0, -1);
        QCOMPARE(entries.at(up).row, lastRow);
        const int down = KeyLayout::neighbor(entries, up, 0, 1);
        QCOMPARE(entries.at(down).row, entries.at(q).row);
        QCOMPARE(down, q);
    }

    void wrappingKeepsTheNearestColumnAndStaysInTheRow()
    {
        const auto entries = KeyLayout::compute(letters(), {1000, 400, 10, false, 0});
        const int a = find(entries, QStringLiteral("a"));
        const int l = find(entries, QStringLiteral("l"));
        QCOMPARE(KeyLayout::neighbor(entries, a, -1, 0), l);
        QCOMPARE(KeyLayout::neighbor(entries, l, 1, 0), a);
        const int top = KeyLayout::neighbor(entries, find(entries, QStringLiteral("p")), 0, -1);
        const int bottom = entries.at(top).row;
        QCOMPARE(bottom, entries.last().row);
        const qreal pCenter = entries.at(find(entries, QStringLiteral("p"))).rect.center().x();
        for (const KeyEntry &entry : entries) {
            if (entry.row == bottom) {
                QVERIFY(std::abs(entries.at(top).rect.center().x() - pCenter) <= std::abs(entry.rect.center().x() - pCenter));
            }
        }
    }

    void splitLayoutWrapsAcrossBothHalves()
    {
        const auto entries = KeyLayout::compute(letters(), {2000, 400, 10, true, 0});
        const int q = find(entries, QStringLiteral("q"), 0);
        const int p = find(entries, QStringLiteral("p"), 1);
        QVERIFY(q >= 0 && p >= 0);
        QCOMPARE(KeyLayout::neighbor(entries, q, -1, 0), p);
        QCOMPARE(KeyLayout::neighbor(entries, p, 1, 0), q);
    }

    void rowWeightsScaleHeights()
    {
        const QVariantMap page = m_layouts.page(QStringLiteral("us"), QStringLiteral("letters"), {{QStringLiteral("numberRow"), true}});
        const auto entries = KeyLayout::compute(page, {1000, 480, 0, false, 0});
        QCOMPARE(entries.at(find(entries, QStringLiteral("1"))).rect.height(), 80.0);
        QCOMPARE(entries.at(find(entries, QStringLiteral("q"))).rect.height(), 100.0);
    }
};

QTEST_GUILESS_MAIN(TestKeyLayout)
#include "testkeylayout.moc"
