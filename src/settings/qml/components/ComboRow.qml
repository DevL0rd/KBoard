pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

SettingRow {
    id: row

    property var options: []

    QQC2.ComboBox {
        model: row.options
        textRole: "label"
        valueRole: "value"
        currentIndex: Math.max(0, row.options.findIndex(option => option.value === row.current))
        onActivated: row.apply(currentValue)
        implicitWidth: Math.max(Kirigami.Units.gridUnit * 10, implicitContentWidth + leftPadding + rightPadding)
        Accessible.name: row.label
    }
}
