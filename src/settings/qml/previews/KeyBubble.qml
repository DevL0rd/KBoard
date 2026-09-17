pragma ComponentBehavior: Bound

import QtQuick
import "Ease.js" as Ease

Item {
    id: bubble
    property var kb
    readonly property var keyInfo: bubble.kb.keyData(bubble.kb.popupKey)
    readonly property rect target: bubble.kb.keyRect(bubble.kb.popupKey)
    visible: keyInfo !== null && keyInfo.kind === "char" && bubble.kb.popup > 0.01
    z: 10
    width: target.width * 1.25
    height: target.height * 1.45
    x: target.x + target.width / 2 - width / 2
    y: target.y - height + target.height * 0.25 - (1 - bubble.kb.popup) * target.height * 0.25
    opacity: Math.min(1, bubble.kb.popup * 1.5)
    scale: 0.85 + 0.15 * bubble.kb.popup
    transformOrigin: Item.Bottom

    Rectangle {
        anchors.fill: parent
        anchors.topMargin: 2
        anchors.bottomMargin: -2
        radius: Math.min(bubble.kb.radiusPx * 1.3 + 2, width / 2)
        color: Qt.alpha("black", 0.25)
    }

    Rectangle {
        anchors.fill: parent
        radius: Math.min(bubble.kb.radiusPx * 1.3 + 2, width / 2)
        color: Ease.mix(bubble.kb.keyColor, "white", bubble.kb.darkTheme ? 0.08 : 0.4)
        border.color: Qt.alpha(bubble.kb.accent, 0.6)
        border.width: 1

        Text {
            anchors.centerIn: parent
            anchors.verticalCenterOffset: -parent.height * 0.1
            text: bubble.keyInfo ? (bubble.kb.shifted ? bubble.keyInfo.label.toUpperCase() : bubble.keyInfo.label) : ""
            color: bubble.kb.textColor
            font.pixelSize: Math.max(7, bubble.target.height * 0.62 * bubble.kb.labelScale)
        }
    }
}
