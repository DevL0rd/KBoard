pragma ComponentBehavior: Bound

import QtQuick

Item {
    id: ripple

    property bool pressed
    property point origin
    property real radius
    property real progress: 0
    property real strength: 0

    readonly property real startSize: Math.min(width, height) * 0.3

    function lerp(from, to) {
        return from + (to - from) * progress
    }

    visible: enabled && strength > 0

    Rectangle {
        x: ripple.lerp(ripple.origin.x - ripple.startSize / 2, 0)
        y: ripple.lerp(ripple.origin.y - ripple.startSize / 2, 0)
        width: ripple.lerp(ripple.startSize, ripple.width)
        height: ripple.lerp(ripple.startSize, ripple.height)
        radius: ripple.lerp(ripple.startSize / 2, ripple.radius)
        color: Theme.ripple
        opacity: ripple.strength
    }

    onPressedChanged: {
        if (!enabled)
            return
        if (pressed) {
            fade.stop()
            strength = 1
            progress = 0
            grow.restart()
        } else {
            fade.restart()
        }
    }

    NumberAnimation {
        id: grow

        target: ripple
        property: "progress"
        to: 1
        duration: Theme.duration(260)
        easing.type: Easing.OutCubic
    }

    SequentialAnimation {
        id: fade

        PauseAnimation {
            duration: grow.running ? Theme.duration(90) : 0
        }
        NumberAnimation {
            target: ripple
            property: "strength"
            to: 0
            duration: Theme.duration(260)
            easing.type: Easing.OutQuad
        }
    }
}
