pragma ComponentBehavior: Bound

import QtQuick
import org.kde.kirigami as Kirigami
import "KeyboardModel.js" as KeyboardModel
import "Ease.js" as Ease

Item {
    id: accentStrip
    property var kb
    readonly property rect target: accentStrip.kb.keyRect(accentStrip.kb.accentKey)
    readonly property var choices: KeyboardModel.accents[accentStrip.kb.accentKey] || []
    readonly property real cell: target.width * 1.05
    visible: choices.length > 0 && accentStrip.kb.accentOpen > 0.01
    z: 11
    width: cell * choices.length + 6 * accentStrip.kb.metric
    height: target.height * 1.2
    x: Math.max(0, Math.min(accentStrip.kb.width - width, target.x + target.width / 2 - width / 2))
    y: target.y - height - 3 * accentStrip.kb.metric

    Rectangle {
        anchors.fill: parent
        radius: Math.min(accentStrip.kb.radiusPx * 1.3 + 3, height / 2)
        color: Ease.mix(accentStrip.kb.keyColor, "white", accentStrip.kb.darkTheme ? 0.06 : 0.35)
        border.color: Qt.alpha(accentStrip.kb.textColor, 0.18)
        opacity: Math.min(1, accentStrip.kb.accentOpen * 2)
        scale: 0.9 + 0.1 * Ease.outBack(Math.min(1, accentStrip.kb.accentOpen * 1.4))
    }

    Row {
        anchors.centerIn: parent

        Repeater {
            model: accentStrip.choices

            Item {
                id: choice
                required property string modelData
                required property int index
                readonly property real appear: Ease.clamp(accentStrip.kb.accentOpen * accentStrip.choices.length * 0.8 - index * 0.35, 0, 1)
                width: accentStrip.cell
                height: accentStrip.height - 6 * accentStrip.kb.metric
                opacity: appear
                transform: Translate {
                    y: (1 - Ease.outCubic(choice.appear)) * accentStrip.height * 0.25
                }

                Rectangle {
                    anchors.fill: parent
                    anchors.margins: Math.max(1, 1.5 * accentStrip.kb.metric)
                    radius: Math.min(accentStrip.kb.radiusPx, height / 2)
                    color: accentStrip.kb.accentIndex === parent.index ? accentStrip.kb.accent : "transparent"
                }

                Text {
                    anchors.centerIn: parent
                    text: parent.modelData
                    color: accentStrip.kb.accentIndex === parent.index ? Kirigami.Theme.highlightedTextColor : accentStrip.kb.textColor
                    font.pixelSize: Math.max(6, accentStrip.target.height * 0.52 * accentStrip.kb.labelScale)
                }
            }
        }
    }
}
