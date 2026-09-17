pragma ComponentBehavior: Bound

import QtQuick

Item {
    id: backdrop

    property real halfWidth: width / 2
    property real radius: Theme.radius * 1.4

    Repeater {
        model: 2

        delegate: Rectangle {
            required property int index

            readonly property bool leading: index === 0

            x: leading ? 0 : backdrop.width - backdrop.halfWidth
            width: backdrop.halfWidth
            height: backdrop.height
            color: Theme.panel
            topLeftRadius: leading ? 0 : backdrop.width - backdrop.halfWidth * 2 > 1 ? backdrop.radius : 0
            topRightRadius: leading && backdrop.width - backdrop.halfWidth * 2 > 1 ? backdrop.radius : 0

            Rectangle {
                width: parent.width
                height: 1
                color: Theme.panelEdge
            }
        }
    }
}
