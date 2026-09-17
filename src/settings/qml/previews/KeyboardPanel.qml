pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Effects

Item {
    id: panelLayer
    property var kb
    anchors.fill: parent

    Repeater {
        model: [0, 1]

        Item {
            id: half
            required property int index
            readonly property real x0: index === 0 ? 0 : (panelLayer.kb.split > 0.001 ? panelLayer.kb.rightEdge : panelLayer.kb.width / 2)
            readonly property real x1: index === 0 ? (panelLayer.kb.split > 0.001 ? panelLayer.kb.leftEdge : panelLayer.kb.width / 2) : panelLayer.kb.width
            x: x0 - panelLayer.kb.gapPx / 2
            width: x1 - x0 + panelLayer.kb.gapPx
            y: -panelLayer.kb.gapPx / 2
            height: panelLayer.kb.height + panelLayer.kb.gapPx
            clip: true

            ShaderEffectSource {
                id: blurSource
                visible: false
                live: true
                sourceItem: panelLayer.kb.backdrop
                sourceRect: panelLayer.kb.backdrop ? Qt.rect(panelLayer.kb.backdropOrigin.x + half.x, panelLayer.kb.backdropOrigin.y + half.y, half.width, half.height) : Qt.rect(0, 0, 0, 0)
            }

            MultiEffect {
                anchors.fill: parent
                visible: panelLayer.kb.blur && panelLayer.kb.backdrop !== null
                source: blurSource
                blurEnabled: true
                blur: 1
                blurMax: 32
                saturation: 0.15
            }

            Rectangle {
                anchors.fill: parent
                radius: panelLayer.kb.split > 0.001 ? Math.max(panelLayer.kb.radiusPx * 1.4, 4 * panelLayer.kb.metric) : 0
                color: Qt.alpha(panelLayer.kb.keyStyle === 3 ? Qt.darker(panelLayer.kb.panelColor, 1.15) : panelLayer.kb.panelColor, panelLayer.kb.keyStyle === 3 ? panelLayer.kb.backgroundOpacity * 0.72 : panelLayer.kb.backgroundOpacity)
                border.width: panelLayer.kb.keyStyle === 3 ? 1 : 0
                border.color: Qt.alpha("white", 0.12)
            }

            Rectangle {
                visible: panelLayer.kb.split < 0.001
                anchors.top: parent.top
                width: parent.width
                height: 1
                color: Qt.alpha(panelLayer.kb.textColor, 0.12)
            }
        }
    }
}
