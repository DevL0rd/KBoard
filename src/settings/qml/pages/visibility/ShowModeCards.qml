pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.devl0rd.kboard.settings

SettingRow {
    id: row

    wideControl: true

    ColumnLayout {
        width: parent.width
        spacing: Kirigami.Units.smallSpacing

        Kirigami.InlineMessage {
            Layout.fillWidth: true
            type: Kirigami.MessageType.Error
            visible: SystemStatus.virtualKeyboardError.length > 0
            text: SystemStatus.virtualKeyboardError
        }

        ChoiceCards {
            Layout.fillWidth: true
            enabled: SystemStatus.kwinReachable
            cardHeight: Kirigami.Units.gridUnit * 4
            currentValue: SystemStatus.virtualKeyboardMode
            options: [
                { value: SystemStatus.Never, title: "Never", description: "Only open it from the panel widget or a shortcut" },
                { value: SystemStatus.TouchOnly, title: "When you touch", description: "Tapping a text field with a finger opens it" },
                { value: SystemStatus.AnyInput, title: "Touch or mouse", description: "Clicking a text field also opens it" }
            ]
            preview: ShowModeIcon {}
            onChosen: value => SystemStatus.virtualKeyboardMode = value
        }
    }
}
