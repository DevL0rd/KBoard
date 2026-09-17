pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.devl0rd.kboard.config
import org.devl0rd.kboard.settings
import "Ease.js" as Ease

RowLayout {
    id: preview

    readonly property var sound: Modules.sound ? Modules.sound.api : null
    readonly property var packs: sound ? sound.packs : []
    readonly property var pack: packs.find(entry => entry.id === Settings.soundPack) || null
    readonly property string caption: !Settings.soundEnabled ? "Key sounds are off" : (pack ? pack.name + " · " + pack.description : "Sound pack “" + Settings.soundPack + "” isn't installed")
    readonly property real variation: Settings.soundVariation ? Math.sin(typing.cycle * 12.9898 + typing.keyIndex * 78.233) : 0
    readonly property real buzz: Settings.haptics && typing.press > 0 ? Math.sin(typing.local * Math.PI * 24) * typing.press : 0

    spacing: Kirigami.Units.gridUnit

    TypingLoop {
        id: typing
        keys: ["t", "a", "p", "space", "t", "a", "p"]
        keyMs: 420
        restMs: 700
    }

    CloseUp {
        Layout.fillHeight: true
        Layout.preferredWidth: height * 1.5
        centerKey: "g"
        rowsVisible: 4.2
        topFraction: 0.22
        transform: Translate {
            x: preview.buzz * 2
        }
        keyboard.showHints: false
        keyboard.pressedKey: typing.pressedKey
        keyboard.press: typing.press
        keyboard.rippleKey: Settings.ripple ? typing.pressedKey : ""
        keyboard.ripple: typing.ripple
    }

    ColumnLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true
        spacing: Kirigami.Units.smallSpacing

        Waveform {
            Layout.fillWidth: true
            Layout.fillHeight: true
            amplitude: Settings.soundEnabled ? Settings.soundVolume : 0
            frequency: 10 + preview.variation * 2.5 + (preview.pack ? preview.pack.name.length % 5 : 0)
            decay: 5 + preview.variation
            energy: 0.85 + preview.variation * 0.15
            progress: typing.pressedKey.length > 0 ? Ease.outCubic(Math.min(1, typing.local * 1.6)) : 1
            lineWidth: 2.5
            opacity: typing.pressedKey.length > 0 ? 1 : 0.35
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: Kirigami.Units.largeSpacing

            Kirigami.Icon {
                source: Settings.soundEnabled ? (Settings.soundVolume > 0.66 ? "audio-volume-high" : (Settings.soundVolume > 0.33 ? "audio-volume-medium" : "audio-volume-low")) : "audio-volume-muted"
                Layout.preferredWidth: Kirigami.Units.iconSizes.small
                Layout.preferredHeight: Kirigami.Units.iconSizes.small
            }

            QQC2.ProgressBar {
                Layout.fillWidth: true
                from: 0
                to: 1
                value: Settings.soundEnabled ? Settings.soundVolume : 0
            }

            Kirigami.Icon {
                source: "smartphone"
                visible: Settings.haptics
                Layout.preferredWidth: Kirigami.Units.iconSizes.small
                Layout.preferredHeight: Kirigami.Units.iconSizes.small
                rotation: preview.buzz * 8
            }
        }
    }
}
