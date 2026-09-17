pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import org.devl0rd.kboard.platform

Item {
    id: panel

    property bool selecting: false

    signal closeRequested
    signal keyTapped

    function arrow(direction) {
        run((selecting ? "shift+" : "") + direction)
    }

    function run(combo) {
        keyTapped()
        InputContext.shortcut(combo)
    }

    readonly property var actions: [
        { text: qsTr("Select all"), icon: "edit-select-all", combo: "ctrl+a" },
        { text: qsTr("Copy"), icon: "edit-copy", combo: "ctrl+c" },
        { text: qsTr("Cut"), icon: "edit-cut", combo: "ctrl+x" },
        { text: qsTr("Paste"), icon: "edit-paste", combo: "ctrl+v" },
        { text: qsTr("Undo"), icon: "edit-undo", combo: "ctrl+z" },
        { text: qsTr("Redo"), icon: "edit-redo", combo: "ctrl+shift+z" },
        { text: qsTr("Home"), icon: "go-first", combo: "home" },
        { text: qsTr("End"), icon: "go-last", combo: "end" }
    ]

    RowLayout {
        anchors.fill: parent
        anchors.margins: Theme.gap / 2
        spacing: 0

        GridLayout {
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.preferredWidth: 5
            columns: 3
            rowSpacing: 0
            columnSpacing: 0

            Item { Layout.fillWidth: true; Layout.fillHeight: true }
            PanelKey { Layout.fillWidth: true; Layout.fillHeight: true; glyph: "up"; autoRepeat: true; onActivated: panel.arrow("up") }
            Item { Layout.fillWidth: true; Layout.fillHeight: true }
            PanelKey { Layout.fillWidth: true; Layout.fillHeight: true; glyph: "left"; autoRepeat: true; onActivated: panel.arrow("left") }
            PanelKey {
                Layout.fillWidth: true
                Layout.fillHeight: true
                iconName: "edit-select-text"
                text: qsTr("Select")
                checked: panel.selecting
                special: true
                onActivated: {
                    panel.keyTapped()
                    panel.selecting = !panel.selecting
                }
            }
            PanelKey { Layout.fillWidth: true; Layout.fillHeight: true; glyph: "right"; autoRepeat: true; onActivated: panel.arrow("right") }
            Item { Layout.fillWidth: true; Layout.fillHeight: true }
            PanelKey { Layout.fillWidth: true; Layout.fillHeight: true; glyph: "down"; autoRepeat: true; onActivated: panel.arrow("down") }
            Item { Layout.fillWidth: true; Layout.fillHeight: true }
        }

        GridLayout {
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.preferredWidth: 6
            columns: 3
            rowSpacing: 0
            columnSpacing: 0

            Repeater {
                model: panel.actions

                delegate: PanelKey {
                    required property var modelData

                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    text: modelData.text
                    iconName: modelData.icon
                    special: true
                    onActivated: panel.run(modelData.combo)
                }
            }

            PanelKey {
                Layout.fillWidth: true
                Layout.fillHeight: true
                glyph: "backspace"
                autoRepeat: true
                special: true
                onActivated: {
                    panel.keyTapped()
                    InputContext.backspace()
                }
            }
        }
    }
}
