pragma ComponentBehavior: Bound

import QtQuick
import org.devl0rd.kboard.config
import org.devl0rd.kboard.settings

SettingsPage {
    preview: LayoutPreview {}

    Section {
        title: "Layouts"

        LayoutListEditor {
            label: "Your layouts"
            description: "Swipe the space bar or tap the globe key to switch between these. The selected one is used first."
            iconName: "input-keyboard"
            setting: "layouts"
        }
    }

    Section {
        title: "Extra rows"

        SwitchRow {
            label: "Number row"
            description: "Always show 1 to 0 above the letters"
            iconName: "format-number-percent"
            setting: "showNumberRow"
        }

        SwitchRow {
            label: "Full keyboard on big screens"
            description: "On wide screens, use the desktop layout with a number row and function keys"
            iconName: "input-keyboard"
            setting: "fullKeyboard"
        }

        SwitchRow {
            label: "Desktop row"
            description: "Esc, Tab, Ctrl, Alt and arrow keys for desktop apps and terminals"
            iconName: "utilities-terminal"
            setting: "showDesktopRow"
        }
    }

    Section {
        title: "Split keyboard"

        ChoiceRow {
            label: "Split into thumb halves"
            description: "Automatic splits on wide screens where full-width keys would be much wider than they are tall"
            iconName: "view-split-left-right"
            setting: "splitMode"
            options: [
                { value: 0, label: "Automatic" },
                { value: 1, label: "Always" },
                { value: 2, label: "Never" }
            ]
        }

        SliderRow {
            label: "Width of each half"
            iconName: "distribute-horizontal"
            setting: "splitHalfWidth"
            enabled: Settings.splitMode !== 2
            from: 0.25
            to: 0.48
            stepSize: 0.01
            displayFactor: 100
            unit: "%"
        }
    }
}
