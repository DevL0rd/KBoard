pragma ComponentBehavior: Bound

import QtQuick

Item {
    id: shadow

    property real radius: 8
    property real spread: 6
    property real offsetY: 3
    property color color: Theme.bubbleShadow

    Repeater {
        model: 4

        delegate: Rectangle {
            required property int index

            readonly property real grow: shadow.spread * (index + 1) / 4

            x: -grow
            y: -grow + shadow.offsetY
            width: shadow.width + grow * 2
            height: shadow.height + grow * 2
            radius: shadow.radius + grow
            color: shadow.color
            opacity: 0.22 / (index + 1)
        }
    }
}
