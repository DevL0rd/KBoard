pragma ComponentBehavior: Bound

import QtQuick
import org.kde.kirigami as Kirigami
import org.devl0rd.kboard.config
import org.devl0rd.kboard.settings

SettingsPage {
    id: page

    preview: LookPreview {}

    Section {
        title: "Size"

        SliderRow {
            label: "Height in landscape"
            description: "How much of a landscape screen the keyboard covers"
            iconName: "view-fullscreen"
            setting: "heightLandscape"
            from: 0.2
            to: 0.6
            stepSize: 0.01
            displayFactor: 100
            unit: "%"
        }

        SliderRow {
            label: "Height in portrait"
            description: "How much of a portrait screen the keyboard covers"
            iconName: "phone"
            setting: "heightPortrait"
            from: 0.15
            to: 0.5
            stepSize: 0.01
            displayFactor: 100
            unit: "%"
        }
    }

    Section {
        title: "Keys"

        CardsRow {
            label: "Key style"
            description: "How each key is drawn"
            setting: "keyStyle"
            cardHeight: Kirigami.Units.gridUnit * 5.5
            options: [
                { value: 0, title: "Flat", description: "Soft fills, no edges" },
                { value: 1, title: "Bordered", description: "Thin outlines" },
                { value: 2, title: "Raised", description: "Keycaps with depth" },
                { value: 3, title: "Glass", description: "Frosted and see-through" }
            ]
            preview: KeySample {}
        }

        SliderRow {
            label: "Corner roundness"
            iconName: "transform-crop"
            setting: "keyRadius"
            from: 0
            to: 32
            unit: "px"
        }

        SliderRow {
            label: "Space between keys"
            iconName: "distribute-horizontal-x"
            setting: "keyGap"
            from: 0
            to: 20
            unit: "px"
        }

        SliderRow {
            label: "Label size"
            iconName: "format-font-size-more"
            setting: "labelScale"
            from: 0.6
            to: 1.6
            stepSize: 0.05
            displayFactor: 100
            unit: "%"
        }
    }

    Section {
        title: "Background"

        SwitchRow {
            label: "Blur behind the keyboard"
            description: "Frost whatever is behind the keyboard so keys stay readable"
            iconName: "blurfx"
            setting: "blurEnabled"
        }

        SliderRow {
            label: "Background opacity"
            description: "Lower values let more of the screen show through"
            iconName: "edit-opacity"
            setting: "backgroundOpacity"
            from: 0.3
            to: 1
            stepSize: 0.01
            displayFactor: 100
            unit: "%"
        }
    }

    Section {
        title: "Colour"

        ChoiceRow {
            label: "Accent colour"
            description: "Used for the enter key, pressed keys, trails and highlights. Light and dark follow your Plasma colour scheme."
            iconName: "color-management"
            setting: "accentMode"
            options: [
                { value: 0, label: "System accent" },
                { value: 1, label: "Custom" }
            ]
        }

        ColorRow {
            label: "Custom accent"
            iconName: "color-picker"
            setting: "accentColor"
            enabled: Settings.accentMode === 1
        }
    }

    Section {
        title: "Motion"

        SwitchRow {
            label: "Animations"
            description: "Key presses, pop-ups and panel transitions. Also used by the previews in this window."
            iconName: "preferences-desktop-effects"
            setting: "animationsEnabled"
        }

        SliderRow {
            label: "Animation speed"
            iconName: "speedometer"
            setting: "animationSpeed"
            enabled: Settings.animationsEnabled
            from: 0.25
            to: 3
            stepSize: 0.05
            decimals: 2
            unit: "×"
            lowLabel: "Slower"
            highLabel: "Faster"
        }
    }
}
