pragma ComponentBehavior: Bound

import QtQuick

Item {
    id: ring

    property rect target
    property real radius: 8

    x: target.x - 3
    y: target.y - 3
    width: target.width + 6
    height: target.height + 6
    z: 10

    Behavior on x {
        enabled: ring.visible
        NumberAnimation { duration: Theme.duration(160); easing.type: Easing.OutCubic }
    }
    Behavior on y {
        enabled: ring.visible
        NumberAnimation { duration: Theme.duration(160); easing.type: Easing.OutCubic }
    }
    Behavior on width {
        NumberAnimation { duration: Theme.duration(160); easing.type: Easing.OutCubic }
    }
    Behavior on height {
        NumberAnimation { duration: Theme.duration(160); easing.type: Easing.OutCubic }
    }

    Rectangle {
        anchors.fill: parent
        anchors.margins: -4
        radius: ring.radius + 7
        color: "transparent"
        border.width: 6
        border.color: Qt.alpha(Theme.accent, 0.22)
    }

    Rectangle {
        anchors.fill: parent
        radius: ring.radius + 3
        color: Qt.alpha(Theme.accent, 0.1)
        border.width: 2.5
        border.color: Theme.accent

        SequentialAnimation on opacity {
            loops: Animation.Infinite
            running: ring.visible && Theme.animated

            NumberAnimation { to: 0.7; duration: 700; easing.type: Easing.InOutSine }
            NumberAnimation { to: 1; duration: 700; easing.type: Easing.InOutSine }
        }
    }
}
