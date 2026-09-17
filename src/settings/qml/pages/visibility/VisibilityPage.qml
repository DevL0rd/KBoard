pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import org.devl0rd.kboard.settings

SettingsPage {
    preview: VisibilityPreview {}

    Section {
        title: "Opening"

        ShowModeCards {
            label: "Open the keyboard automatically"
            description: "This is KWin's virtual keyboard setting, shared with System Settings"
            iconName: "input-keyboard-virtual"
        }

        SettingRow {
            label: "Virtual keyboard in use"
            description: SystemStatus.kboardIsInputMethod ? "KBoard is the virtual keyboard KWin starts" : (SystemStatus.inputMethod.length > 0 ? "KWin is set to start " + SystemStatus.inputMethod : "No virtual keyboard is selected in System Settings")
            iconName: SystemStatus.kboardIsInputMethod ? "checkmark" : "dialog-warning"

            QQC2.Button {
                icon.name: "configure"
                text: "System Settings…"
                onClicked: SystemStatus.openVirtualKeyboardSettings()
            }
        }
    }

    Section {
        title: "Hiding"

        SwitchRow {
            label: "Swipe down to hide"
            description: "Drag down on an empty spot or the top edge of the keyboard to close it"
            iconName: "go-down-skip"
            setting: "hideOnSwipeDown"
        }
    }

    Section {
        title: "Apps"

        AppRulesEditor {
            label: "App rules"
            description: "Change how the keyboard behaves in specific apps"
            iconName: "preferences-system-windows-actions"
            setting: "appRules"
        }
    }
}
