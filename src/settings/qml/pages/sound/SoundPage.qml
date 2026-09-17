pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.devl0rd.kboard.config
import org.devl0rd.kboard.settings

SettingsPage {
    id: page

    readonly property var sound: Modules.sound ? Modules.sound.api : null

    preview: SoundPreview {}

    ModuleNotice {
        module: "sound"
        what: "Key sounds"
    }

    Kirigami.InlineMessage {
        Layout.fillWidth: true
        Layout.maximumWidth: Kirigami.Units.gridUnit * 46
        Layout.alignment: Qt.AlignHCenter
        type: Kirigami.MessageType.Warning
        visible: page.sound !== null && page.sound.errorString.length > 0
        text: page.sound ? page.sound.errorString : ""
    }

    Section {
        title: "Sound"

        SwitchRow {
            label: "Key sounds"
            description: "Play a short sound for every key press"
            iconName: "audio-volume-high"
            setting: "soundEnabled"
        }

        CardsRow {
            label: "Sound pack"
            description: "Press play to hear a pack before choosing it"
            iconName: "folder-sound"
            setting: "soundPack"
            enabled: Settings.soundEnabled
            cardHeight: Kirigami.Units.gridUnit * 5
            options: (page.sound ? page.sound.packs : []).map(pack => ({ value: pack.id, title: pack.name, description: pack.description }))
            preview: PackCard {}
        }

        SliderRow {
            label: "Volume"
            description: "Separate from your system volume, and silent when the system is muted"
            iconName: "audio-volume-medium"
            setting: "soundVolume"
            enabled: Settings.soundEnabled
            from: 0
            to: 1
            stepSize: 0.01
            displayFactor: 100
            unit: "%"
        }

        SwitchRow {
            label: "Natural variation"
            description: "Nudge pitch and loudness a little on each press so typing never sounds robotic"
            iconName: "games-config-custom"
            setting: "soundVariation"
            enabled: Settings.soundEnabled
        }
    }

    Section {
        title: "Haptics"

        SwitchRow {
            label: "Vibrate on key press"
            description: page.sound && !page.sound.hapticsAvailable ? "This device has no vibration motor that KBoard can use" : "A short buzz on devices with a vibration motor"
            iconName: "smartphone"
            setting: "haptics"
        }
    }
}
