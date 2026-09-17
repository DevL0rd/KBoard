pragma ComponentBehavior: Bound

import QtQuick
import org.kde.kirigami as Kirigami

Item {
    id: wave

    readonly property int bars: Math.max(12, Math.floor(width / (Kirigami.Units.smallSpacing * 2.2)))
    property var history: []
    property real elapsed: 0

    function push(value, dt) {
        elapsed += dt
        if (elapsed < 1 / 30)
            return
        elapsed = 0
        const next = history.slice(Math.max(0, history.length - bars + 1))
        next.push(value)
        history = next
    }

    Row {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        spacing: Kirigami.Units.smallSpacing * 0.9

        Repeater {
            model: wave.bars

            Rectangle {
                required property int index
                readonly property int historyIndex: wave.history.length - wave.bars + index
                readonly property real value: historyIndex >= 0 ? wave.history[historyIndex] : 0

                anchors.verticalCenter: parent.verticalCenter
                width: Kirigami.Units.smallSpacing * 1.3
                height: Math.max(width, wave.height * (0.12 + value * 0.88))
                radius: width / 2
                color: VoiceStyle.accent
                opacity: 0.25 + 0.75 * (index / wave.bars)
            }
        }
    }
}
