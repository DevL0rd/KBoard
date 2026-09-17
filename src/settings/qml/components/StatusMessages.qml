pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.devl0rd.kboard.settings

ColumnLayout {
    id: root

    spacing: Kirigami.Units.smallSpacing

    Kirigami.InlineMessage {
        id: failure
        Layout.fillWidth: true
        Layout.leftMargin: Kirigami.Units.largeSpacing
        Layout.rightMargin: Kirigami.Units.largeSpacing
        type: Kirigami.MessageType.Error
        showCloseButton: true
        visible: false

        Connections {
            target: SettingsStore
            function onFailed(message) {
                failure.text = message;
                failure.visible = true;
            }
        }
    }

    Kirigami.InlineMessage {
        Layout.fillWidth: true
        Layout.leftMargin: Kirigami.Units.largeSpacing
        Layout.rightMargin: Kirigami.Units.largeSpacing
        type: Kirigami.MessageType.Error
        visible: SettingsCatalog.errorString.length > 0
        text: SettingsCatalog.errorString
    }

    Kirigami.InlineMessage {
        Layout.fillWidth: true
        Layout.leftMargin: Kirigami.Units.largeSpacing
        Layout.rightMargin: Kirigami.Units.largeSpacing
        type: Kirigami.MessageType.Warning
        visible: SystemStatus.inputMethod.length > 0 && !SystemStatus.kboardIsInputMethod
        text: "KWin is using a different on-screen keyboard right now. Pick KBoard in System Settings to use it."
        actions: [
            Kirigami.Action {
                icon.name: "configure"
                text: "Open Virtual Keyboard settings"
                onTriggered: SystemStatus.openVirtualKeyboardSettings()
            }
        ]
    }

    Kirigami.InlineMessage {
        Layout.fillWidth: true
        Layout.leftMargin: Kirigami.Units.largeSpacing
        Layout.rightMargin: Kirigami.Units.largeSpacing
        type: Kirigami.MessageType.Information
        visible: SystemStatus.kboardIsInputMethod && !SystemStatus.keyboardRunning
        text: "KBoard isn't running right now. Your changes are saved and apply when it starts."
    }
}
