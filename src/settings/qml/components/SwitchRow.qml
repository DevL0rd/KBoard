pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2

SettingRow {
    id: row

    property bool inverted: false
    readonly property bool isOn: inverted ? !current : current === true

    onClicked: toggle.toggle()

    QQC2.Switch {
        id: toggle
        checked: row.isOn
        onToggled: row.apply(row.inverted ? !checked : checked)
        Accessible.name: row.label
    }
}
