pragma ComponentBehavior: Bound

import QtQuick

Item {
    id: flash

    property string word
    property real progress: 0

    function play(text) {
        word = text
        animation.restart()
    }

    visible: progress > 0 && progress < 1
    z: 5

    Rectangle {
        anchors.fill: parent
        color: Theme.panelSolid
        opacity: Math.min(1, (1 - flash.progress) * 3)
    }

    Text {
        id: label

        anchors.centerIn: parent
        text: flash.word
        color: Theme.accent
        font.family: Theme.font.family
        font.pixelSize: Math.round(flash.height * 0.4 * Theme.labelScale)
        font.weight: Font.DemiBold
        opacity: Math.min(1, (1 - flash.progress) * 3)
    }

    Rectangle {
        anchors.top: label.bottom
        anchors.horizontalCenter: label.horizontalCenter
        height: Math.max(2, flash.height * 0.05)
        radius: height / 2
        width: label.width * Math.min(1, flash.progress * 3)
        color: Theme.accent
        opacity: label.opacity
    }

    NumberAnimation {
        id: animation

        target: flash
        property: "progress"
        from: 0
        to: 1
        duration: Theme.animated ? Theme.duration(900) : 1
    }
}
