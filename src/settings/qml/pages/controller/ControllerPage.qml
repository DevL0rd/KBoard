pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.devl0rd.kboard.config
import org.devl0rd.kboard.settings

SettingsPage {
    id: page

    readonly property var pad: Modules.gamepad ? Modules.gamepad.api : null

    preview: ControllerPreview {}

    ModuleNotice {
        module: "gamepad"
        what: "Controller support"
    }

    Kirigami.InlineMessage {
        Layout.fillWidth: true
        Layout.maximumWidth: Kirigami.Units.gridUnit * 46
        Layout.alignment: Qt.AlignHCenter
        type: Kirigami.MessageType.Error
        visible: page.pad !== null && page.pad.errorString.length > 0
        text: page.pad ? page.pad.errorString : ""
    }

    Section {
        title: "Controller"

        SwitchRow {
            label: "Type with a controller"
            description: "Move between keys with the D-pad or left stick and press them with A"
            iconName: "input-gamepad"
            setting: "controllerEnabled"
        }

        SettingRow {
            label: "Connected controller"
            description: page.pad && page.pad.connected ? page.pad.name : "No controller is connected right now"
            iconName: page.pad && page.pad.connected ? "network-connect" : "network-disconnect"

            QQC2.Button {
                icon.name: "notifications"
                text: "Test rumble"
                enabled: page.pad !== null && page.pad.connected && page.pad.rumbleSupported && Settings.controllerRumble
                onClicked: page.pad.rumble(0.7, 350)
            }
        }

        ChordEditor {
            label: "Open the keyboard with"
            description: page.pad && page.pad.chordError.length > 0 ? page.pad.chordError : "Hold these buttons together to open the keyboard in any text field"
            iconName: "input-keyboard-virtual"
            setting: "controllerOpenChord"
            enabled: Settings.controllerEnabled
        }

        SwitchRow {
            label: "Rumble"
            description: "A short buzz when keys are pressed and panels open"
            iconName: "notifications"
            setting: "controllerRumble"
            enabled: Settings.controllerEnabled
        }

        SwitchRow {
            label: "Radial typing with both sticks"
            description: "Each stick picks letters from a ring, which is faster once you're used to it"
            iconName: "office-chart-pie"
            setting: "controllerRadialMode"
            enabled: Settings.controllerEnabled
        }

        SwitchRow {
            label: "Keep controller input for the keyboard"
            description: "While the keyboard is open, the game or app underneath doesn't also receive your button presses"
            iconName: "object-locked"
            setting: "controllerConsumeInput"
            enabled: Settings.controllerEnabled
        }

        SliderRow {
            label: "Focus speed"
            description: "How quickly focus repeats while you hold a direction"
            iconName: "speedometer"
            setting: "controllerStickSpeed"
            enabled: Settings.controllerEnabled
            from: 0.25
            to: 3
            stepSize: 0.05
            decimals: 2
            unit: "×"
        }

        SliderRow {
            label: "Stick dead zone"
            description: "How far a stick has to move before it counts"
            iconName: "crosshairs"
            setting: "controllerDeadzone"
            enabled: Settings.controllerEnabled
            from: 0.1
            to: 0.9
            stepSize: 0.05
            displayFactor: 100
            unit: "%"
        }
    }

    Section {
        title: "Buttons"

        ActionTable {
            label: "Button actions"
            description: "What each button does while the keyboard is open"
        }
    }
}
