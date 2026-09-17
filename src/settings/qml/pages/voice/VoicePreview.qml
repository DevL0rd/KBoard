pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.devl0rd.kboard.config
import org.devl0rd.kboard.settings

RowLayout {
    id: preview

    readonly property var voice: Modules.voice ? Modules.voice.api : null
    readonly property bool live: voice !== null && voice.state === "listening"
    readonly property var words: Settings.voiceCommands ? ["Meet", "me", "at", "the", "station", "at", "five", "period"] : ["Meet", "me", "at", "the", "station", "at", "five"]
    readonly property int spoken: Math.floor(clock.progress("talk") * words.length + (clock.step === "talk" ? 0 : 0))
    readonly property real level: live ? voice.level : (clock.step === "talk" ? 0.35 + 0.45 * Math.abs(Math.sin(clock.now / 90)) * Math.abs(Math.sin(clock.now / 37)) : 0.05)
    readonly property string sentence: {
        const said = words.slice(0, spoken);
        if (said.length === words.length && Settings.voiceCommands) {
            return said.slice(0, -1).join(" ") + ".";
        }
        return said.join(" ");
    }
    readonly property string caption: voice ? (voice.modelName.length > 0 ? voice.modelName + " " + voice.modelVariant + (voice.backendLabel.length > 0 ? " on " + voice.backendLabel : (voice.modelDownloaded ? " · loads when you start talking" : " · not downloaded yet")) : "No speech model selected") : "Voice typing isn't available"

    spacing: Kirigami.Units.gridUnit

    SceneClock {
        id: clock
        steps: [{ id: "pause", ms: 600 }, { id: "talk", ms: 2600 }, { id: "hold", ms: 1500 }, { id: "rest", ms: 500 }]
    }

    Item {
        Layout.fillHeight: true
        Layout.preferredWidth: height
        Layout.maximumWidth: Kirigami.Units.gridUnit * 9

        Repeater {
            model: 3

            Rectangle {
                required property int index
                anchors.centerIn: parent
                width: parent.height * (0.45 + 0.18 * (index + 1) * preview.level)
                height: width
                radius: width / 2
                color: "transparent"
                border.width: 2
                border.color: Qt.alpha(Kirigami.Theme.highlightColor, 0.5 - index * 0.14)

                Behavior on width {
                    NumberAnimation {
                        duration: Motion.quick
                        easing.type: Easing.OutCubic
                    }
                }
            }
        }

        Rectangle {
            anchors.centerIn: parent
            width: parent.height * 0.42
            height: width
            radius: width / 2
            color: Kirigami.Theme.highlightColor

            Kirigami.Icon {
                anchors.centerIn: parent
                width: parent.width * 0.5
                height: width
                source: "audio-input-microphone-symbolic"
                color: Kirigami.Theme.highlightedTextColor
                isMask: true
            }
        }
    }

    ColumnLayout {
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignVCenter
        spacing: Kirigami.Units.largeSpacing

        TextLine {
            Layout.fillWidth: true
            text: preview.live ? preview.voice.partialText : preview.sentence
            fontSize: Kirigami.Units.gridUnit * 1.05
            placeholder: "Listening…"
        }

        Flow {
            Layout.fillWidth: true
            spacing: Kirigami.Units.smallSpacing
            visible: Settings.voiceCommands

            Repeater {
                model: ["new line", "period", "comma", "question mark", "delete that"]

                Kirigami.Chip {
                    required property string modelData
                    text: "“" + modelData + "”"
                    closable: false
                    checkable: false
                    checked: modelData === "period" && preview.spoken >= preview.words.length
                }
            }
        }

        QQC2.Label {
            visible: !Settings.voiceCommands
            text: "Voice commands are off, so words like “period” are typed as words"
            opacity: 0.7
            font: Kirigami.Theme.smallFont
        }
    }
}
