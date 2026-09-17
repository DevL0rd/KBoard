import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents
import "lib"

Item {
    id: full

    readonly property real margin: Kirigami.Units.largeSpacing

    Layout.minimumWidth: Kirigami.Units.gridUnit * 16
    Layout.preferredWidth: Kirigami.Units.gridUnit * 20
    Layout.minimumHeight: layout.implicitHeight + margin * 2
    Layout.preferredHeight: Layout.minimumHeight
    Layout.maximumHeight: Layout.minimumHeight

    ColumnLayout {
        id: layout
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: full.margin
        spacing: Kirigami.Units.largeSpacing

        RowLayout {
            Layout.fillWidth: true
            spacing: Kirigami.Units.largeSpacing

            Item {
                Layout.preferredWidth: Kirigami.Units.iconSizes.medium
                Layout.preferredHeight: Kirigami.Units.iconSizes.medium
                Kirigami.Icon {
                    anchors.fill: parent
                    source: "input-keyboard-virtual"
                }
                Rectangle {
                    width: Math.round(Kirigami.Units.iconSizes.medium * 0.34)
                    height: width
                    radius: width / 2
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    anchors.margins: -2
                    color: root.statusColor
                    border.width: 2
                    border.color: Kirigami.Theme.backgroundColor
                    Behavior on color { ColorAnimation { duration: Kirigami.Units.longDuration } }
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 0
                Kirigami.Heading {
                    Layout.fillWidth: true
                    level: 3
                    text: i18n("KBoard")
                    font.weight: Font.DemiBold
                    elide: Text.ElideRight
                }
                PlasmaComponents.Label {
                    Layout.fillWidth: true
                    text: root.statusText
                    font: Kirigami.Theme.smallFont
                    opacity: 0.65
                    elide: Text.ElideRight
                }
            }

            PlasmaComponents.ToolButton {
                icon.name: "configure"
                display: PlasmaComponents.AbstractButton.IconOnly
                text: i18n("KBoard settings")
                enabled: root.running
                onClicked: root.openSettings("")
                QQC2.ToolTip.visible: hovered
                QQC2.ToolTip.text: text
            }
        }

        PopCard {
            id: problem
            visible: !root.ready && root.status !== "checking"
            title: root.status === "inactive" ? i18n("Not in use") : root.status === "stopped" ? i18n("Not running") : i18n("Something went wrong")
            icon: "dialog-warning"
            trailing: ""
            border.color: Qt.alpha(root.statusColor, 0.45)
            color: Qt.alpha(root.statusColor, 0.08)

            PlasmaComponents.Label {
                Layout.fillWidth: true
                wrapMode: Text.Wrap
                text: root.status === "inactive"
                    ? i18n("KWin is using %1 as the on-screen keyboard, so KBoard can't show up.", root.previousInputMethodName)
                    : root.status === "stopped"
                    ? i18n("KWin has KBoard set as the on-screen keyboard, but it isn't running. Restart it to bring it back.")
                    : root.statusText
            }
            RowLayout {
                Layout.fillWidth: true
                PlasmaComponents.BusyIndicator {
                    visible: root.switching
                    Layout.preferredWidth: Kirigami.Units.iconSizes.smallMedium
                    Layout.preferredHeight: Kirigami.Units.iconSizes.smallMedium
                }
                Item { Layout.fillWidth: true }
                PlasmaComponents.Button {
                    visible: root.status === "error"
                    icon.name: "view-refresh"
                    text: i18n("Try again")
                    onClicked: root.retry()
                }
                PlasmaComponents.Button {
                    visible: root.status !== "error"
                    enabled: !root.switching
                    icon.name: root.status === "inactive" ? "input-keyboard-virtual-on" : "view-refresh"
                    text: root.status === "inactive" ? i18n("Use KBoard") : i18n("Restart KBoard")
                    onClicked: root.useKBoard()
                }
            }
        }

        MouseArea {
            id: hero
            Layout.fillWidth: true
            implicitHeight: heroRow.implicitHeight + Kirigami.Units.largeSpacing * 2.5
            enabled: root.ready
            opacity: enabled ? 1 : 0.5
            hoverEnabled: true
            cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
            activeFocusOnTab: true
            Keys.onReturnPressed: root.toggle()
            Keys.onSpacePressed: root.toggle()
            onClicked: root.toggle()

            Rectangle {
                anchors.fill: parent
                radius: Kirigami.Units.cornerRadius * 3
                color: root.keyboardVisible
                    ? Qt.alpha(Kirigami.Theme.highlightColor, hero.pressed ? 0.42 : hero.containsMouse ? 0.34 : 0.26)
                    : Qt.alpha(hero.containsMouse || hero.activeFocus ? Kirigami.Theme.highlightColor : Kirigami.Theme.textColor, hero.pressed ? 0.24 : hero.containsMouse || hero.activeFocus ? 0.12 : 0.06)
                border.width: 1
                border.color: root.keyboardVisible || hero.activeFocus ? Qt.alpha(Kirigami.Theme.highlightColor, 0.85) : Qt.alpha(Kirigami.Theme.textColor, 0.08)
                scale: hero.pressed ? 0.98 : 1
                Behavior on color { ColorAnimation { duration: Kirigami.Units.longDuration } }
                Behavior on border.color { ColorAnimation { duration: Kirigami.Units.longDuration } }
                Behavior on scale { NumberAnimation { duration: Kirigami.Units.shortDuration; easing.type: Easing.OutCubic } }
            }

            RowLayout {
                id: heroRow
                anchors.fill: parent
                anchors.leftMargin: Kirigami.Units.largeSpacing * 1.5
                anchors.rightMargin: Kirigami.Units.largeSpacing * 1.5
                spacing: Kirigami.Units.largeSpacing * 1.5

                Item {
                    Layout.preferredWidth: Kirigami.Units.iconSizes.large
                    Layout.preferredHeight: Kirigami.Units.iconSizes.large

                    Rectangle {
                        anchors.fill: parent
                        radius: width / 2
                        color: Qt.alpha(Kirigami.Theme.highlightColor, root.keyboardVisible ? 0.3 : 0.14)
                        Behavior on color { ColorAnimation { duration: Kirigami.Units.longDuration } }
                    }
                    Ripple {
                        anchors.fill: parent
                    }
                    Kirigami.Icon {
                        anchors.centerIn: parent
                        width: Kirigami.Units.iconSizes.medium
                        height: width
                        source: root.keyboardVisible ? "input-keyboard-virtual-hide" : "input-keyboard-virtual-show"
                        fallback: "input-keyboard-virtual"
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 0
                    PlasmaComponents.Label {
                        text: root.keyboardVisible ? i18n("SHOWING") : i18n("HIDDEN")
                        font.pixelSize: Kirigami.Theme.smallFont.pixelSize
                        font.weight: Font.DemiBold
                        font.letterSpacing: 0.6
                        opacity: 0.6
                    }
                    PlasmaComponents.Label {
                        Layout.fillWidth: true
                        text: root.keyboardVisible ? i18n("Hide keyboard") : i18n("Show keyboard")
                        font.pixelSize: Kirigami.Theme.defaultFont.pixelSize * 1.3
                        font.weight: Font.DemiBold
                        elide: Text.ElideRight
                    }
                }

                Kirigami.Icon {
                    Layout.preferredWidth: Kirigami.Units.iconSizes.small
                    Layout.preferredHeight: Kirigami.Units.iconSizes.small
                    source: root.keyboardVisible ? "go-down" : "go-up"
                    opacity: hero.containsMouse ? 0.9 : 0.45
                    Behavior on opacity { NumberAnimation { duration: Kirigami.Units.shortDuration } }
                }
            }
        }

        PopCard {
            title: i18n("Jump to")

            GridLayout {
                Layout.fillWidth: true
                columns: 3
                columnSpacing: Kirigami.Units.smallSpacing * 2
                rowSpacing: Kirigami.Units.smallSpacing * 2

                Repeater {
                    model: [
                        { panel: "emoji", label: i18n("Emoji"), icon: "face-smile", hint: i18n("Open the emoji picker") },
                        { panel: "gif", label: i18n("GIFs"), icon: "image-gif", hint: i18n("Search and send GIFs") },
                        { panel: "clipboard", label: i18n("Clipboard"), icon: "edit-paste", hint: i18n("Paste from your clipboard history") },
                        { panel: "voice", label: i18n("Voice"), icon: "audio-input-microphone", hint: i18n("Type with your voice") },
                        { panel: "edit", label: i18n("Edit"), icon: "edit-select-text", hint: i18n("Move the cursor, select, copy and paste") }
                    ]

                    ActionTile {
                        required property var modelData
                        label: modelData.label
                        iconName: modelData.icon
                        hint: modelData.hint
                        enabled: root.ready
                        current: root.keyboardVisible && root.currentPanel === modelData.panel
                        onActivated: root.openPanel(modelData.panel)
                    }
                }

                ActionTile {
                    label: i18n("Settings")
                    iconName: "configure"
                    hint: i18n("Open KBoard settings")
                    enabled: root.running
                    onActivated: root.openSettings("")
                }
            }
        }
    }
}
