pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.devl0rd.kboard.settings
import "ControllerActions.js" as ControllerActions
import "Glyphs.js" as Glyphs

SettingRow {
    id: row

    readonly property var pad: Modules.gamepad ? Modules.gamepad.api : null

    wideControl: true

    GridLayout {
        width: parent.width
        columns: width > Kirigami.Units.gridUnit * 30 ? 2 : 1
        columnSpacing: Kirigami.Units.gridUnit
        rowSpacing: Kirigami.Units.smallSpacing

        Repeater {
            model: ControllerActions.actions

            RowLayout {
                id: action
                required property var modelData
                spacing: Kirigami.Units.largeSpacing
                Layout.fillWidth: true

                ButtonGlyph {
                    Layout.minimumWidth: Kirigami.Units.gridUnit * 2.5
                    text: action.modelData.group === "dpad" ? "D-pad" : Glyphs.label(row.pad, action.modelData.button)
                    active: row.pad !== null && (action.modelData.group === "dpad" ? ["dpup", "dpdown", "dpleft", "dpright"].some(button => row.pad.heldButtons.indexOf(button) >= 0) : row.pad.heldButtons.indexOf(action.modelData.button) >= 0)
                }

                QQC2.Label {
                    text: action.modelData.action
                    Layout.fillWidth: true
                    elide: Text.ElideRight
                }
            }
        }
    }
}
