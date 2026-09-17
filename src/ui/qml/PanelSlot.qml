pragma ComponentBehavior: Bound

import QtQuick

Item {
    id: slot

    required property string name
    required property string current
    property Component component
    property real travel: 24
    readonly property bool shown: current === name
    readonly property alias item: loader.item

    visible: opacity > 0
    opacity: shown ? 1 : 0
    enabled: shown
    transform: Translate {
        y: (1 - slot.opacity) * (slot.shown ? slot.travel : -slot.travel * 0.5)
    }

    Behavior on opacity {
        NumberAnimation {
            duration: Theme.duration(slot.shown ? 220 : 140)
            easing.type: slot.shown ? Easing.OutCubic : Easing.InQuad
        }
    }

    Loader {
        id: loader

        anchors.fill: parent
        active: slot.component !== null && (slot.shown || slot.opacity > 0)
        sourceComponent: slot.component
    }
}
