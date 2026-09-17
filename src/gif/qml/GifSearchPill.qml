import QtQuick
import org.kde.kirigami as Kirigami

Rectangle {
    id: pill

    property string text
    property string placeholder
    property bool active: false
    property real animationScale: 1

    signal clicked()
    signal clearClicked()

    radius: height / 2
    color: Qt.alpha(Kirigami.Theme.textColor, active ? 0.1 : (tap.pressed ? 0.14 : 0.07))
    border.width: active ? 2 : 0
    border.color: Kirigami.Theme.highlightColor

    Behavior on color {
        ColorAnimation {
            duration: Kirigami.Units.shortDuration * pill.animationScale
        }
    }

    Kirigami.Icon {
        id: searchIcon
        anchors.left: parent.left
        anchors.leftMargin: Kirigami.Units.largeSpacing
        anchors.verticalCenter: parent.verticalCenter
        width: Kirigami.Units.iconSizes.small
        height: width
        source: "search"
        color: pill.active ? Kirigami.Theme.highlightColor : Qt.alpha(Kirigami.Theme.textColor, 0.7)
        isMask: true
    }

    Item {
        anchors.left: searchIcon.right
        anchors.right: clearButton.visible ? clearButton.left : parent.right
        anchors.leftMargin: Kirigami.Units.smallSpacing * 1.5
        anchors.rightMargin: Kirigami.Units.largeSpacing
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        clip: true

        Text {
            id: label
            anchors.verticalCenter: parent.verticalCenter
            x: Math.min(0, parent.width - width - caret.width - 2)
            text: pill.text !== "" ? pill.text : pill.placeholder
            color: pill.text !== "" ? Kirigami.Theme.textColor : Qt.alpha(Kirigami.Theme.textColor, 0.55)
            font.pointSize: Kirigami.Theme.defaultFont.pointSize
        }

        Rectangle {
            id: caret
            visible: pill.active
            x: pill.text !== "" ? label.x + label.width + 1 : 0
            anchors.verticalCenter: parent.verticalCenter
            width: 2
            height: label.implicitHeight
            radius: 1
            color: Kirigami.Theme.highlightColor

            SequentialAnimation on opacity {
                running: pill.active && pill.animationScale > 0
                loops: Animation.Infinite
                NumberAnimation { from: 1; to: 0; duration: 500; easing.type: Easing.InOutQuad }
                NumberAnimation { from: 0; to: 1; duration: 500; easing.type: Easing.InOutQuad }
            }
        }
    }

    GifRoundButton {
        id: clearButton
        anchors.right: parent.right
        anchors.rightMargin: Kirigami.Units.smallSpacing
        anchors.verticalCenter: parent.verticalCenter
        width: parent.height - Kirigami.Units.smallSpacing * 2
        height: width
        visible: pill.text !== ""
        iconName: "edit-clear"
        idleOpacity: 0
        animationScale: pill.animationScale
        onClicked: pill.clearClicked()
    }

    TapHandler {
        id: tap
        onTapped: pill.clicked()
    }
}
