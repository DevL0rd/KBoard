pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.devl0rd.kboard.settings
import "Ease.js" as Ease

Item {
    id: card

    property var choice: null
    readonly property string packId: choice ? choice.value : ""
    readonly property int seed: packId.split("").reduce((sum, c) => sum + c.charCodeAt(0), 0)
    property real playback: 1

    NumberAnimation {
        id: play
        target: card
        property: "playback"
        from: 0
        to: 1
        duration: Motion.enabled ? 700 : 0
        easing.type: Easing.OutCubic
    }

    Connections {
        target: Modules.sound ? Modules.sound.api : null
        function onPlayed(kind) {
            if (Modules.sound.api.currentPack === card.packId && kind === "key") {
                play.restart();
            }
        }
    }

    RowLayout {
        anchors.fill: parent
        spacing: Kirigami.Units.smallSpacing

        Waveform {
            Layout.fillWidth: true
            Layout.fillHeight: true
            frequency: 5 + card.seed % 9
            decay: 3 + card.seed % 5
            progress: card.playback
            energy: 0.35 + 0.65 * Ease.pulse(Math.min(1, card.playback * 1.2))
            lineWidth: 1.5
        }

        QQC2.ToolButton {
            icon.name: "media-playback-start"
            text: "Play " + (card.choice ? card.choice.title : "")
            display: QQC2.AbstractButton.IconOnly
            enabled: Modules.sound !== null
            onClicked: {
                Modules.sound.api.preview(card.packId);
                play.restart();
            }
            QQC2.ToolTip.text: text
            QQC2.ToolTip.visible: hovered
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
        }
    }
}
