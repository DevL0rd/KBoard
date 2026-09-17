#include "layouts.h"

#include <QTest>

class TestLayouts : public QObject
{
    Q_OBJECT

private:
    Layouts m_layouts {QStringLiteral(KBOARD_LAYOUT_DIR)};

    static double rowUnits(const QVariantMap &row)
    {
        double units = 0;
        for (const QVariant &key : row.value(QStringLiteral("keys")).toList()) {
            units += key.toMap().value(QStringLiteral("width")).toDouble();
        }
        return units;
    }

    static QStringList types(const QVariantMap &row)
    {
        QStringList result;
        for (const QVariant &key : row.value(QStringLiteral("keys")).toList()) {
            result.append(key.toMap().value(QStringLiteral("type")).toString());
        }
        return result;
    }

private Q_SLOTS:
    void loadsEveryFileWithoutErrors()
    {
        QCOMPARE(m_layouts.errorString(), QString());
        const QStringList expected {QStringLiteral("colemak"), QStringLiteral("de"), QStringLiteral("dvorak"), QStringLiteral("es"),
            QStringLiteral("fr"), QStringLiteral("uk"), QStringLiteral("us")};
        for (const QString &id : expected) {
            QVERIFY2(m_layouts.has(id), qPrintable(id));
        }
        QCOMPARE(m_layouts.pages(),
            QStringList({QStringLiteral("numpad"), QStringLiteral("phone"), QStringLiteral("symbols"), QStringLiteral("symbols-more")}));
    }

    void everyPageResolvesForEveryLayout()
    {
        for (const QVariant &entry : m_layouts.available()) {
            const QString id = entry.toMap().value(QStringLiteral("id")).toString();
            for (const QString &page : QStringList {QStringLiteral("letters")} + m_layouts.pages()) {
                const QVariantMap resolved
                    = m_layouts.page(id, page, {{QStringLiteral("numberRow"), true}, {QStringLiteral("desktopRow"), true}});
                const QVariantList rows = resolved.value(QStringLiteral("rows")).toList();
                QVERIFY2(rows.size() >= 4, qPrintable(id + QLatin1Char('/') + page));
                for (const QVariant &row : rows) {
                    QVERIFY(!row.toMap().value(QStringLiteral("keys")).toList().isEmpty());
                }
            }
        }
    }

    void bottomRowGrowsSpaceToFillColumns()
    {
        const QVariantMap page = m_layouts.page(QStringLiteral("us"), QStringLiteral("letters"));
        const QVariantMap bottom = page.value(QStringLiteral("rows")).toList().constLast().toMap();
        QCOMPARE(rowUnits(bottom), 10.0);
        QVERIFY(types(bottom).contains(QStringLiteral("emoji")));
        QVERIFY(!types(bottom).contains(QStringLiteral("globe")));
    }

    void multipleLayoutsShowGlobe()
    {
        const QVariantMap page
            = m_layouts.page(QStringLiteral("us"), QStringLiteral("letters"), {{QStringLiteral("multipleLayouts"), true}});
        const QStringList bottom = types(page.value(QStringLiteral("rows")).toList().constLast().toMap());
        QVERIFY(bottom.contains(QStringLiteral("globe")));
        QVERIFY(!bottom.contains(QStringLiteral("emoji")));
    }

    void variantsReplaceTheBottomRow()
    {
        const QVariantMap page
            = m_layouts.page(QStringLiteral("us"), QStringLiteral("letters"), {{QStringLiteral("variant"), QStringLiteral("email")}});
        const QVariantList keys = page.value(QStringLiteral("rows")).toList().constLast().toMap().value(QStringLiteral("keys")).toList();
        QStringList labels;
        for (const QVariant &key : keys) {
            labels.append(key.toMap().value(QStringLiteral("label")).toString());
        }
        QVERIFY(labels.contains(QStringLiteral("@")));
        QVERIFY(labels.contains(QStringLiteral(".com")));
    }

    void optionalRowsArePrepended()
    {
        const QVariantMap page = m_layouts.page(
            QStringLiteral("us"), QStringLiteral("letters"), {{QStringLiteral("numberRow"), true}, {QStringLiteral("desktopRow"), true}});
        const QVariantList rows = page.value(QStringLiteral("rows")).toList();
        QCOMPARE(rows.size(), 6);
        QVERIFY(types(rows.at(0).toMap()).contains(QStringLiteral("modifier")));
        QCOMPARE(rows.at(1).toMap().value(QStringLiteral("keys")).toList().first().toMap().value(QStringLiteral("shiftLabel")).toString(),
            QStringLiteral("!"));
        const QVariantMap function = m_layouts.page(
            QStringLiteral("us"), QStringLiteral("letters"), {{QStringLiteral("desktopRow"), true}, {QStringLiteral("functionRow"), true}});
        QCOMPARE(function.value(QStringLiteral("rows"))
                     .toList()
                     .first()
                     .toMap()
                     .value(QStringLiteral("keys"))
                     .toList()
                     .at(1)
                     .toMap()
                     .value(QStringLiteral("key"))
                     .toString(),
            QStringLiteral("f1"));
    }

    void keysGetDefaults()
    {
        const QVariantMap page = m_layouts.page(QStringLiteral("us"), QStringLiteral("letters"));
        const QVariantList rows = page.value(QStringLiteral("rows")).toList();
        const QVariantMap e = rows.at(0).toMap().value(QStringLiteral("keys")).toList().at(2).toMap();
        QCOMPARE(e.value(QStringLiteral("shiftLabel")).toString(), QStringLiteral("E"));
        QCOMPARE(e.value(QStringLiteral("hint")).toString(), QStringLiteral("3"));
        QVERIFY(e.value(QStringLiteral("glide")).toBool());
        QCOMPARE(e.value(QStringLiteral("shiftLongPress")).toList().at(1).toMap().value(QStringLiteral("output")).toString(),
            QStringLiteral("È"));
        const QVariantMap backspace = rows.at(2).toMap().value(QStringLiteral("keys")).toList().constLast().toMap();
        QVERIFY(backspace.value(QStringLiteral("repeat")).toBool());
        QVERIFY(backspace.value(QStringLiteral("special")).toBool());
    }

    void currencyFollowsTheLayout()
    {
        const QVariantMap page = m_layouts.page(QStringLiteral("de"), QStringLiteral("symbols"));
        const QVariantMap currency
            = page.value(QStringLiteral("rows")).toList().at(1).toMap().value(QStringLiteral("keys")).toList().at(2).toMap();
        QCOMPARE(currency.value(QStringLiteral("label")).toString(), QStringLiteral("€"));
    }

    void unknownLayoutsResolveEmpty()
    {
        QVERIFY(m_layouts.page(QStringLiteral("nope"), QStringLiteral("letters")).isEmpty());
        QVERIFY(m_layouts.page(QStringLiteral("us"), QStringLiteral("nope")).isEmpty());
    }

    void missingDirectoryReportsAnError()
    {
        const Layouts broken(QStringLiteral("/nonexistent/kboard/layouts"));
        QVERIFY(!broken.errorString().isEmpty());
        QVERIFY(broken.available().isEmpty());
    }
};

QTEST_GUILESS_MAIN(TestLayouts)
#include "testlayouts.moc"
