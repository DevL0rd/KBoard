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

    readonly property var store: Modules.emoji ? Modules.emoji.api : null
    readonly property var gifs: Modules.gif ? Modules.gif.api : null
    readonly property var faces: ["👋", "👍", "👏", "🙌", "✌️", "👌", "🤞", "💪", "🙏", "✍️", "🤙", "👊"]
    readonly property var filterNames: ["Off", "Low", "Medium", "High"]
    readonly property real sendT: clock.progress("send")
    readonly property string caption: "Skin tone and GIF " + (Settings.gifInsertMode === 0 ? "images" : "links") + " · safety filter " + filterNames[Settings.gifContentFilter].toLowerCase()

    spacing: Kirigami.Units.gridUnit

    SceneClock {
        id: clock
        steps: [{ id: "pause", ms: 900 }, { id: "tap", ms: 400 }, { id: "send", ms: 1000, ease: "inOutCubic" }, { id: "hold", ms: 1400 }]
    }

    Rectangle {
        Layout.fillHeight: true
        Layout.preferredWidth: height * 1.25
        radius: Kirigami.Units.cornerRadius * 2
        color: Qt.alpha(Kirigami.Theme.backgroundColor, 0.8)
        border.color: Qt.alpha(Kirigami.Theme.textColor, 0.15)

        GridLayout {
            anchors.fill: parent
            anchors.margins: Kirigami.Units.largeSpacing
            columns: 4
            rowSpacing: 0
            columnSpacing: 0

            Repeater {
                model: preview.faces

                QQC2.Label {
                    required property string modelData
                    required property int index
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    text: preview.store ? preview.store.withSkinTone(modelData, Settings.skinTone) : modelData
                    font.pixelSize: Math.max(12, height * 0.55)
                    scale: index === 1 && clock.step === "tap" ? 1 + 0.25 * Ease.pulse(clock.raw("tap")) : 1
                }
            }
        }
    }

    Rectangle {
        Layout.fillHeight: true
        Layout.preferredWidth: height * 1.25
        radius: Kirigami.Units.cornerRadius * 2
        color: Qt.alpha(Kirigami.Theme.backgroundColor, 0.8)
        border.color: Qt.alpha(Kirigami.Theme.textColor, 0.15)
        opacity: Settings.gifEnabled ? 1 : 0.4

        GridLayout {
            id: tiles
            anchors.fill: parent
            anchors.margins: Kirigami.Units.largeSpacing
            anchors.bottomMargin: Kirigami.Units.gridUnit * 1.4
            columns: 3
            rowSpacing: Kirigami.Units.smallSpacing
            columnSpacing: Kirigami.Units.smallSpacing

            Repeater {
                model: 6

                GifTileMock {
                    required property int index
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    seed: index
                    animated: Settings.gifAutoplay
                    phase: clock.time / clock.total
                    pressed: index === 4 && clock.step === "tap" ? Ease.pulse(clock.raw("tap")) : 0
                }
            }
        }

        QQC2.Label {
            anchors.bottom: parent.bottom
            anchors.bottomMargin: Kirigami.Units.smallSpacing
            anchors.horizontalCenter: parent.horizontalCenter
            text: preview.gifs ? preview.gifs.attribution : "GIFs unavailable"
            font: Kirigami.Theme.smallFont
            opacity: 0.7
        }
    }

    ChatMock {
        Layout.fillWidth: true
        Layout.fillHeight: true
        emoji: preview.store ? preview.store.withSkinTone("👍", Settings.skinTone) : "👍"
        linkMode: Settings.gifInsertMode === 1
        arrival: clock.step === "send" ? preview.sendT : (clock.step === "hold" ? 1 : 0)
        phase: clock.time / clock.total
        animated: Settings.gifAutoplay
    }
}
