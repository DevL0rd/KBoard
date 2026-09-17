pragma ComponentBehavior: Bound

import QtQuick
import org.kde.kirigami as Kirigami

Rectangle {
    id: window

    property string text
    property bool focused: false
    property real fieldGlow: 0

    radius: Math.max(2, height * 0.04)
    color: Kirigami.Theme.backgroundColor
    border.color: Qt.alpha(Kirigami.Theme.textColor, 0.2)

    Rectangle {
        width: parent.width
        height: Math.max(6, parent.height * 0.13)
        radius: parent.radius
        color: Qt.alpha(Kirigami.Theme.textColor, 0.08)

        Row {
            anchors.right: parent.right
            anchors.rightMargin: parent.height * 0.4
            anchors.verticalCenter: parent.verticalCenter
            spacing: parent.height * 0.3

            Repeater {
                model: 3

                Rectangle {
                    width: window.height * 0.045
                    height: width
                    radius: width / 2
                    color: Qt.alpha(Kirigami.Theme.textColor, 0.3)
                }
            }
        }
    }

    Column {
        x: parent.width * 0.06
        y: parent.height * 0.24
        spacing: parent.height * 0.06

        Repeater {
            model: [0.7, 0.55, 0.62]

            Rectangle {
                required property real modelData
                width: window.width * modelData
                height: Math.max(2, window.height * 0.035)
                radius: height / 2
                color: Qt.alpha(Kirigami.Theme.textColor, 0.15)
            }
        }
    }

    Rectangle {
        id: field
        x: parent.width * 0.06
        width: parent.width * 0.88
        height: Math.max(8, parent.height * 0.2)
        anchors.bottom: parent.bottom
        anchors.bottomMargin: parent.height * 0.08
        radius: Math.max(2, height * 0.25)
        color: Kirigami.Theme.backgroundColor
        border.width: window.focused ? 1.5 : 1
        border.color: window.focused ? Kirigami.Theme.highlightColor : Qt.alpha(Kirigami.Theme.textColor, 0.25)

        Rectangle {
            anchors.fill: parent
            anchors.margins: -3 * window.fieldGlow
            radius: parent.radius + 3
            color: "transparent"
            border.width: 2
            border.color: Qt.alpha(Kirigami.Theme.highlightColor, 0.5 * window.fieldGlow)
            visible: window.fieldGlow > 0.01
        }

        Text {
            id: fieldText
            anchors.verticalCenter: parent.verticalCenter
            x: parent.height * 0.35
            text: window.text
            color: Kirigami.Theme.textColor
            font.pixelSize: Math.max(5, parent.height * 0.5)
        }

        Rectangle {
            visible: window.focused
            x: fieldText.x + fieldText.contentWidth + 1
            anchors.verticalCenter: parent.verticalCenter
            width: 1
            height: parent.height * 0.55
            color: Kirigami.Theme.highlightColor
        }
    }
}
