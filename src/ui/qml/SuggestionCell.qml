pragma ComponentBehavior: Bound

import QtQuick

Item {
    id: cell

    property string text
    property bool primary: false
    property bool corrected: false
    property string shown: text

    signal clicked
    signal held

    Rectangle {
        anchors.fill: parent
        anchors.margins: parent.height * 0.1
        radius: height / 2
        color: Qt.alpha(Theme.text, 0.1)
        opacity: area.pressed && cell.shown !== "" ? 1 : 0

        Behavior on opacity {
            NumberAnimation { duration: Theme.duration(90) }
        }
    }

    Text {
        id: label

        anchors.centerIn: parent
        width: Math.min(implicitWidth, parent.width - 16)
        text: cell.shown
        elide: Text.ElideRight
        color: cell.primary && cell.corrected ? Theme.accent : Theme.stripText
        font.family: Theme.font.family
        font.pixelSize: Math.round(cell.height * 0.4 * Theme.labelScale)
        font.weight: cell.primary ? Font.DemiBold : Font.Normal
    }

    onTextChanged: swap.restart()

    SequentialAnimation {
        id: swap

        NumberAnimation { target: label; property: "opacity"; to: 0; duration: Theme.duration(50) }
        ScriptAction { script: cell.shown = cell.text }
        ParallelAnimation {
            NumberAnimation { target: label; property: "opacity"; to: 1; duration: Theme.duration(130); easing.type: Easing.OutCubic }
            NumberAnimation { target: label; property: "anchors.verticalCenterOffset"; from: cell.height * 0.08; to: 0; duration: Theme.duration(160); easing.type: Easing.OutCubic }
        }
    }

    MouseArea {
        id: area

        anchors.fill: parent
        enabled: cell.shown !== ""
        onClicked: cell.clicked()
        onPressAndHold: cell.held()
    }
}
