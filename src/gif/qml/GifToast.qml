import QtQuick
import org.kde.kirigami as Kirigami

Rectangle {
    id: toast

    property string text
    property bool shown: false
    property real maximumWidth: Kirigami.Units.gridUnit * 20
    property real animationScale: 1

    signal dismissed()

    width: Math.min(maximumWidth, row.implicitWidth + Kirigami.Units.gridUnit * 1.5)
    height: Kirigami.Units.gridUnit * 2
    radius: height / 2
    color: Kirigami.Theme.negativeBackgroundColor
    border.width: 1
    border.color: Qt.alpha(Kirigami.Theme.negativeTextColor, 0.4)
    opacity: shown ? 1 : 0
    scale: shown ? 1 : 0.9
    visible: opacity > 0

    Behavior on opacity {
        NumberAnimation { duration: Kirigami.Units.longDuration * toast.animationScale; easing.type: Easing.OutCubic }
    }
    Behavior on scale {
        NumberAnimation { duration: Kirigami.Units.longDuration * toast.animationScale; easing.type: Easing.OutBack }
    }

    Row {
        id: row
        anchors.centerIn: parent
        spacing: Kirigami.Units.smallSpacing

        Kirigami.Icon {
            anchors.verticalCenter: parent.verticalCenter
            width: Kirigami.Units.iconSizes.small
            height: width
            source: "dialog-error"
            color: Kirigami.Theme.negativeTextColor
            isMask: true
        }

        Text {
            anchors.verticalCenter: parent.verticalCenter
            width: Math.min(implicitWidth, toast.maximumWidth - Kirigami.Units.gridUnit * 3)
            elide: Text.ElideRight
            text: toast.text
            color: Kirigami.Theme.textColor
        }
    }

    TapHandler {
        onTapped: toast.dismissed()
    }
}
