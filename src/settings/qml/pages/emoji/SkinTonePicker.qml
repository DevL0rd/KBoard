pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.devl0rd.kboard.settings

SettingRow {
    id: row

    readonly property var store: Modules.emoji ? Modules.emoji.api : null
    readonly property var names: ["Default", "Light", "Medium-light", "Medium", "Medium-dark", "Dark"]

    RowLayout {
        spacing: Kirigami.Units.smallSpacing

        Repeater {
            model: 6

            QQC2.ToolButton {
                id: tone
                required property int index
                readonly property bool selected: row.current === index
                checkable: true
                checked: selected
                enabled: row.store !== null
                text: row.store ? row.store.withSkinTone("👋", index) : ""
                font.pixelSize: Kirigami.Units.gridUnit * 1.2
                implicitWidth: Kirigami.Units.gridUnit * 2.3
                implicitHeight: implicitWidth
                onClicked: row.apply(index)
                Accessible.name: row.names[index] + " skin tone"
                QQC2.ToolTip.text: row.names[index]
                QQC2.ToolTip.visible: hovered
                QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
            }
        }
    }
}
