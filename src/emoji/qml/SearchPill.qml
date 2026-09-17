import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

MouseArea {
    id: pill

    property string text
    property bool active: false
    property string placeholder: "Search emoji"
    property real animScale: 1

    signal cleared()

    cursorShape: Qt.IBeamCursor
    hoverEnabled: true

    Rectangle {
        anchors.fill: parent
        radius: height / 2
        color: Qt.alpha(Kirigami.Theme.textColor, pill.active ? 0.1 : (pill.containsMouse ? 0.1 : 0.07))
        border.width: pill.active ? 2 : 0
        border.color: Kirigami.Theme.highlightColor
        scale: pill.pressed && !pill.active ? 0.98 : 1
        Behavior on color { ColorAnimation { duration: Math.round(140 * pill.animScale) } }
        Behavior on scale { NumberAnimation { duration: Math.round(160 * pill.animScale); easing.type: Easing.OutCubic } }
    }

    Kirigami.Icon {
        id: searchIcon
        anchors.left: parent.left
        anchors.leftMargin: Math.round(pill.height * 0.32)
        anchors.verticalCenter: parent.verticalCenter
        width: Math.round(pill.height * 0.44)
        height: width
        source: "search"
        color: pill.active ? Kirigami.Theme.highlightColor : Kirigami.Theme.textColor
        opacity: pill.active ? 1 : 0.7
    }

    Item {
        id: field
        anchors.left: searchIcon.right
        anchors.leftMargin: Math.round(pill.height * 0.22)
        anchors.right: clearButton.visible ? clearButton.left : parent.right
        anchors.rightMargin: Math.round(pill.height * 0.3)
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        clip: true

        QQC2.Label {
            id: placeholderLabel
            anchors.verticalCenter: parent.verticalCenter
            x: pill.active ? caret.width + 2 : 0
            width: parent.width
            visible: pill.text === ""
            text: pill.placeholder
            elide: Text.ElideRight
            color: Kirigami.Theme.textColor
            opacity: 0.55
            font.pixelSize: Math.round(pill.height * 0.38)
        }

        QQC2.Label {
            id: typed
            anchors.verticalCenter: parent.verticalCenter
            x: Math.min(0, field.width - implicitWidth - caret.width - 2)
            text: pill.text
            color: Kirigami.Theme.textColor
            font.pixelSize: Math.round(pill.height * 0.38)
        }

        Rectangle {
            id: caret
            visible: pill.active
            x: pill.text === "" ? 0 : typed.x + typed.implicitWidth + 1
            anchors.verticalCenter: parent.verticalCenter
            width: 2
            height: Math.round(pill.height * 0.5)
            radius: 1
            color: Kirigami.Theme.highlightColor

            SequentialAnimation on opacity {
                running: pill.active && pill.animScale > 0
                loops: Animation.Infinite
                alwaysRunToEnd: false
                NumberAnimation { to: 1; duration: 0 }
                PauseAnimation { duration: 520 }
                NumberAnimation { to: 0; duration: 180; easing.type: Easing.InOutQuad }
                PauseAnimation { duration: 300 }
                NumberAnimation { to: 1; duration: 180; easing.type: Easing.InOutQuad }
            }
        }
    }

    MouseArea {
        id: clearButton
        visible: pill.active && pill.text !== ""
        anchors.right: parent.right
        anchors.rightMargin: Math.round(pill.height * 0.12)
        anchors.verticalCenter: parent.verticalCenter
        width: Math.round(pill.height * 0.76)
        height: width
        onClicked: pill.cleared()

        Rectangle {
            anchors.fill: parent
            radius: width / 2
            color: Qt.alpha(Kirigami.Theme.textColor, clearButton.pressed ? 0.18 : 0)
        }

        Kirigami.Icon {
            anchors.centerIn: parent
            width: Math.round(parent.width * 0.55)
            height: width
            source: "dialog-close"
            color: Kirigami.Theme.textColor
        }
    }
}
