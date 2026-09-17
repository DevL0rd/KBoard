pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

SettingRow {
    id: row

    property real from: 0
    property real to: 100
    property real stepSize: 1
    property int decimals: 0
    property real displayFactor: 1
    property string unit
    property string lowLabel
    property string highLabel
    property var formatter: null

    function format(number) {
        if (row.formatter) {
            return row.formatter(number);
        }
        return Number(number * row.displayFactor).toFixed(row.decimals) + (row.unit.length ? (row.unit === "%" || row.unit === "×" ? "" : " ") + row.unit : "");
    }

    RowLayout {
        spacing: Kirigami.Units.smallSpacing

        QQC2.Label {
            text: row.lowLabel
            visible: text.length > 0
            opacity: 0.7
            font: Kirigami.Theme.smallFont
        }

        QQC2.Slider {
            id: slider
            from: row.from
            to: row.to
            value: row.current !== undefined ? Number(row.current) : row.from
            Layout.preferredWidth: Kirigami.Units.gridUnit * 10
            onMoved: row.apply(Math.round(value / row.stepSize) * row.stepSize)
            Accessible.name: row.label
        }

        QQC2.Label {
            text: row.highLabel
            visible: text.length > 0
            opacity: 0.7
            font: Kirigami.Theme.smallFont
        }

        QQC2.Label {
            text: row.format(Math.round(slider.value / row.stepSize) * row.stepSize)
            horizontalAlignment: Text.AlignRight
            font.features: { "tnum": 1 }
            Layout.minimumWidth: Kirigami.Units.gridUnit * 3.5
        }
    }
}
