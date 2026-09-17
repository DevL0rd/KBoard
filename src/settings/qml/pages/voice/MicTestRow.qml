pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.devl0rd.kboard.settings

SettingRow {
    id: row

    readonly property var voice: Modules.voice ? Modules.voice.api : null
    readonly property bool listening: voice !== null && (voice.state === "listening" || voice.state === "processing" || voice.state === "loading")
    readonly property string transcript: voice ? (voice.committedText + (voice.partialText.length > 0 ? " " + voice.partialText : "")).trim() : ""

    wideControl: true

    ColumnLayout {
        width: parent.width
        spacing: Kirigami.Units.smallSpacing

        RowLayout {
            spacing: Kirigami.Units.largeSpacing
            Layout.fillWidth: true

            QQC2.Button {
                icon.name: row.listening ? "media-playback-stop" : "audio-input-microphone"
                text: row.listening ? "Stop" : "Start mic test"
                enabled: row.voice !== null && row.voice.state !== "needs-model"
                onClicked: row.listening ? row.voice.stop() : row.voice.start()
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 6
                radius: 3
                color: Qt.alpha(Kirigami.Theme.textColor, 0.12)

                Rectangle {
                    width: parent.width * (row.voice ? Math.min(1, row.voice.level) : 0)
                    height: parent.height
                    radius: 3
                    color: Kirigami.Theme.positiveTextColor

                    Behavior on width {
                        NumberAnimation {
                            duration: Motion.quick
                        }
                    }
                }
            }

            QQC2.Label {
                text: row.voice ? row.voice.state : ""
                opacity: 0.7
                font: Kirigami.Theme.smallFont
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: Math.max(Kirigami.Units.gridUnit * 3, said.implicitHeight + Kirigami.Units.largeSpacing * 2)
            radius: Kirigami.Units.cornerRadius
            color: Qt.alpha(Kirigami.Theme.textColor, 0.05)
            border.color: row.listening ? Kirigami.Theme.highlightColor : Qt.alpha(Kirigami.Theme.textColor, 0.12)

            QQC2.Label {
                id: said
                anchors.fill: parent
                anchors.margins: Kirigami.Units.largeSpacing
                wrapMode: Text.Wrap
                text: {
                    if (!row.voice) {
                        return "Voice typing isn't available: " + Modules.errorFor("voice");
                    }
                    if (row.voice.errorString.length > 0) {
                        return row.voice.errorString;
                    }
                    if (row.voice.state === "needs-model") {
                        return "Download a speech model above to try voice typing.";
                    }
                    return row.transcript.length > 0 ? row.transcript : "Press Start and say something. Nothing is typed into other apps during the test.";
                }
                color: row.voice && row.voice.errorString.length > 0 ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.textColor
                opacity: row.transcript.length > 0 ? 1 : 0.7
            }
        }
    }
}
