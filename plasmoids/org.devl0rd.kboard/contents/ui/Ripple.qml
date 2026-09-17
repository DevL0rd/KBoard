import QtQuick
import org.kde.kirigami as Kirigami

Rectangle {
    id: ripple

    property color tint: Kirigami.Theme.highlightColor

    radius: width / 2
    color: "transparent"
    border.width: Math.max(1.5, width * 0.06)
    border.color: tint
    opacity: 0
    scale: 0.4

    function play() {
        flash.restart()
    }

    Connections {
        target: root
        function onTriggered() {
            ripple.play()
        }
    }

    ParallelAnimation {
        id: flash
        NumberAnimation { target: ripple; property: "scale"; from: 0.4; to: 1.15; duration: Kirigami.Units.veryLongDuration; easing.type: Easing.OutCubic }
        NumberAnimation { target: ripple; property: "opacity"; from: 0.9; to: 0; duration: Kirigami.Units.veryLongDuration; easing.type: Easing.OutCubic }
    }
}
