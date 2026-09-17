pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.devl0rd.kboard.settings
import "Glyphs.js" as Glyphs

SettingRow {
    id: row

    readonly property var pad: Modules.gamepad ? Modules.gamepad.api : null
    property bool capturing: false
    property var captured: []

    Connections {
        target: row.capturing ? row.pad : null
        ignoreUnknownSignals: true
        function onButtonPressed(button) {
            if (row.captured.indexOf(button) < 0) {
                row.captured = row.captured.concat([button]);
            }
        }
        function onButtonReleased(button) {
            if (row.pad.heldButtons.length === 0 && row.captured.length > 0) {
                row.apply(row.captured.join("+"));
                row.capturing = false;
            }
        }
    }

    RowLayout {
        spacing: Kirigami.Units.smallSpacing

        Repeater {
            model: row.capturing ? row.captured : String(row.current || "").split("+").filter(Boolean)

            ButtonGlyph {
                required property string modelData
                text: Glyphs.label(row.pad, modelData)
                active: row.capturing
            }
        }

        QQC2.Label {
            visible: row.capturing && row.captured.length === 0
            text: "Press the buttons now…"
            color: Kirigami.Theme.highlightColor
        }

        QQC2.Button {
            text: row.capturing ? "Cancel" : "Change"
            icon.name: row.capturing ? "dialog-cancel" : "input-gamepad"
            enabled: row.pad !== null && row.pad.connected
            onClicked: {
                row.captured = [];
                row.capturing = !row.capturing;
            }
            QQC2.ToolTip.text: "Connect a controller to record a new button combination"
            QQC2.ToolTip.visible: hovered && !enabled
        }
    }
}
