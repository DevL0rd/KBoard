pragma ComponentBehavior: Bound

import QtQuick
import org.devl0rd.kboard.config

Item {
    id: bubble

    required property int index
    required property var keyData
    required property string keyType
    required property real keyX
    required property real keyY
    required property real keyWidth
    required property real keyHeight
    required property bool pressed
    required property var keyboard

    readonly property bool eligible: Settings.keyPopup && keyType === "char" && keyboard.popupKeyIndex !== index
    readonly property bool showing: pressed && eligible
    readonly property string text: keyboard.shifted ? keyData.shiftLabel : keyData.label
    property real presence: 0

    readonly property real bubbleWidth: Math.max(keyWidth + keyHeight * 0.28, keyHeight * 0.95)
    readonly property real bubbleHeight: keyHeight * 1.18

    x: Math.max(0, Math.min(keyboard.width - bubbleWidth, keyX + keyWidth / 2 - bubbleWidth / 2))
    y: keyY - bubbleHeight - keyHeight * 0.1 + (1 - presence) * keyHeight * 0.22
    width: bubbleWidth
    height: bubbleHeight
    visible: presence > 0
    opacity: presence
    scale: 0.82 + 0.18 * presence
    transformOrigin: Item.Bottom

    SoftShadow {
        anchors.fill: face
        radius: face.radius
        spread: Math.round(height * 0.12)
        offsetY: Math.round(height * 0.04)
    }

    Rectangle {
        id: face

        anchors.fill: parent
        radius: Math.min(Theme.radius * 1.4, height / 3)
        color: Theme.bubble
        border.width: 1
        border.color: Qt.alpha(Theme.text, Theme.dark ? 0.1 : 0.08)

        Text {
            anchors.centerIn: parent
            anchors.verticalCenterOffset: -parent.height * 0.02
            text: bubble.text
            color: Theme.keyText
            font.family: Theme.font.family
            font.pixelSize: Math.round(bubble.bubbleHeight * 0.5 * Theme.labelScale)
        }
    }

    onShowingChanged: {
        if (showing) {
            hide.stop()
            show.restart()
        } else {
            show.stop()
            hide.restart()
        }
    }

    NumberAnimation {
        id: show

        target: bubble
        property: "presence"
        to: 1
        duration: Theme.duration(70)
        easing.type: Easing.OutCubic
    }

    NumberAnimation {
        id: hide

        target: bubble
        property: "presence"
        to: 0
        duration: Theme.duration(140)
        easing.type: Easing.InQuad
    }
}
