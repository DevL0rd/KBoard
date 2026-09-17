pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.devl0rd.kboard.config
import org.devl0rd.kboard.settings

ColumnLayout {
    id: demo

    readonly property string sample: "Hold backspace to delete"
    readonly property real holdMs: Settings.keyRepeatDelay + Settings.keyRepeatInterval * 14
    readonly property real elapsed: clock.step === "hold" ? clock.raw("hold") * holdMs : (clock.step === "rest" ? holdMs : 0)
    readonly property int deleted: elapsed <= 0 ? 0 : 1 + (elapsed < Settings.keyRepeatDelay ? 0 : Math.floor((elapsed - Settings.keyRepeatDelay) / Settings.keyRepeatInterval) + 1)
    readonly property int repeats: Math.max(0, Math.floor((holdMs - Settings.keyRepeatDelay) / Settings.keyRepeatInterval) + 1)

    spacing: Kirigami.Units.smallSpacing

    SceneClock {
        id: clock
        steps: [{ id: "pause", ms: 500 }, { id: "hold", ms: demo.holdMs * Motion.speed }, { id: "rest", ms: 900 }]
        stillTime: 500 + demo.holdMs * Motion.speed * 0.6
    }

    TextLine {
        Layout.fillWidth: true
        text: demo.sample.substring(0, Math.max(0, demo.sample.length - demo.deleted))
        fontSize: Kirigami.Units.gridUnit * 0.8
        implicitHeight: Kirigami.Units.gridUnit * 1.8
    }

    Item {
        Layout.fillWidth: true
        Layout.preferredHeight: Kirigami.Units.gridUnit * 1.4

        Rectangle {
            anchors.verticalCenter: parent.verticalCenter
            width: parent.width
            height: 2
            color: Qt.alpha(Kirigami.Theme.textColor, 0.15)
        }

        Rectangle {
            anchors.verticalCenter: parent.verticalCenter
            width: parent.width * Math.min(1, demo.elapsed / demo.holdMs)
            height: 2
            color: Settings.accentMode === 1 ? Settings.accentColor : Kirigami.Theme.highlightColor
        }

        Repeater {
            model: demo.repeats + 1

            Rectangle {
                required property int index
                readonly property real at: index === 0 ? 0 : Settings.keyRepeatDelay + (index - 1) * Settings.keyRepeatInterval
                x: parent.width * at / demo.holdMs - width / 2
                anchors.verticalCenter: parent.verticalCenter
                width: index === 0 ? 8 : 5
                height: width
                radius: width / 2
                color: demo.elapsed >= at && demo.elapsed > 0 ? (Settings.accentMode === 1 ? Settings.accentColor : Kirigami.Theme.highlightColor) : Qt.alpha(Kirigami.Theme.textColor, 0.3)
            }
        }
    }

    QQC2.Label {
        Layout.fillWidth: true
        horizontalAlignment: Text.AlignHCenter
        font: Kirigami.Theme.smallFont
        opacity: 0.75
        text: "Starts after " + Settings.keyRepeatDelay + " ms, then every " + Settings.keyRepeatInterval + " ms"
    }
}
