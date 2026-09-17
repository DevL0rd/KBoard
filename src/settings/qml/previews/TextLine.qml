pragma ComponentBehavior: Bound

import QtQuick
import org.kde.kirigami as Kirigami

Rectangle {
    id: line

    property string text
    property int caret: text.length
    property int markStart: -1
    property int markLength: 0
    property real mark: 0
    property color markColor: Kirigami.Theme.highlightColor
    property int selectStart: -1
    property int selectLength: 0
    property string placeholder: "Message"
    property real fontSize: Kirigami.Units.gridUnit * 0.95
    property bool blink: true

    implicitHeight: Kirigami.Units.gridUnit * 2.2
    radius: height / 2
    color: Kirigami.Theme.backgroundColor
    border.color: Qt.alpha(Kirigami.Theme.highlightColor, 0.7)
    border.width: 1.5

    function prefixWidth(count) {
        return metrics.advanceWidth(line.text.substring(0, count));
    }

    FontMetrics {
        id: metrics
        font.pixelSize: line.fontSize
    }

    Item {
        id: content
        anchors.fill: parent
        anchors.leftMargin: line.height * 0.5
        anchors.rightMargin: line.height * 0.5
        clip: true

        readonly property real shift: Math.max(0, line.prefixWidth(line.caret) - width + line.height * 0.3)

        Rectangle {
            visible: line.markStart >= 0 && line.mark > 0
            x: line.prefixWidth(line.markStart) - content.shift - 2
            width: line.prefixWidth(line.markStart + line.markLength) - line.prefixWidth(line.markStart) + 4
            height: label.contentHeight * 1.05
            anchors.verticalCenter: parent.verticalCenter
            radius: 3
            color: Qt.alpha(line.markColor, 0.28 * line.mark)
        }

        Rectangle {
            visible: line.selectStart >= 0 && line.selectLength > 0
            x: line.prefixWidth(line.selectStart) - content.shift
            width: line.prefixWidth(line.selectStart + line.selectLength) - line.prefixWidth(line.selectStart)
            height: label.contentHeight
            anchors.verticalCenter: parent.verticalCenter
            color: Qt.alpha(Kirigami.Theme.highlightColor, 0.45)
        }

        Text {
            id: label
            x: -content.shift
            anchors.verticalCenter: parent.verticalCenter
            text: line.text.length > 0 ? line.text : line.placeholder
            color: Kirigami.Theme.textColor
            opacity: line.text.length > 0 ? 1 : 0.45
            font.pixelSize: line.fontSize
            textFormat: Text.PlainText
        }

        Rectangle {
            id: caretBar
            x: line.prefixWidth(line.caret) - content.shift
            anchors.verticalCenter: parent.verticalCenter
            width: 2
            radius: 1
            height: label.contentHeight * 1.1
            color: Kirigami.Theme.highlightColor

            SequentialAnimation on opacity {
                running: line.blink && Motion.enabled && caretBar.visible
                loops: Animation.Infinite
                NumberAnimation { to: 0.15; duration: Motion.ms(530); easing.type: Easing.InOutQuad }
                NumberAnimation { to: 1; duration: Motion.ms(530); easing.type: Easing.InOutQuad }
            }
        }
    }
}
