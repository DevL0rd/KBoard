pragma ComponentBehavior: Bound

import QtQuick
import org.devl0rd.kboard.settings

SettingsPage {
    preview: PressPreview {}

    Section {
        title: "Tapping"

        SwitchRow {
            label: "Pop-up above pressed keys"
            description: "Show a bigger copy of the letter above your finger"
            iconName: "zoom-in"
            setting: "keyPopup"
        }

        SwitchRow {
            label: "Ripple on touch"
            description: "A soft wave spreads from where you touch the key"
            iconName: "tool_pen"
            setting: "ripple"
        }
    }

    Section {
        title: "Long press"

        SliderRow {
            label: "Long-press delay"
            description: "How long to hold a key before accents and symbols open"
            iconName: "chronometer"
            setting: "longPressDelay"
            from: 150
            to: 1000
            stepSize: 10
            unit: "ms"
            lowLabel: "Quicker"
            highLabel: "Longer"
        }

        SwitchRow {
            label: "Show symbol hints on keys"
            description: "A small label in the corner tells you what long press types, like @ on A"
            iconName: "tag"
            setting: "showKeyHints"
        }
    }

    Section {
        title: "Key repeat"

        SliderRow {
            label: "Repeat delay"
            description: "How long to hold backspace or an arrow before it repeats"
            iconName: "chronometer-start"
            setting: "keyRepeatDelay"
            from: 150
            to: 1000
            stepSize: 10
            unit: "ms"
        }

        SliderRow {
            label: "Repeat interval"
            description: "Time between repeats while you keep holding"
            iconName: "media-playlist-repeat"
            setting: "keyRepeatInterval"
            from: 15
            to: 200
            stepSize: 5
            unit: "ms"
            lowLabel: "Faster"
            highLabel: "Slower"
        }
    }
}
