import QtQuick
import org.kde.kirigami as Kirigami

Item {
    id: status

    property string iconName
    property string title
    property string text
    property string actionText
    property real animationScale: 1

    signal actionTriggered()

    implicitWidth: column.implicitWidth
    implicitHeight: column.implicitHeight

    opacity: visible ? 1 : 0
    Behavior on opacity {
        NumberAnimation { duration: Kirigami.Units.longDuration * status.animationScale; easing.type: Easing.OutCubic }
    }

    Column {
        id: column
        anchors.centerIn: parent
        width: Math.min(parent.width - Kirigami.Units.gridUnit * 2, Kirigami.Units.gridUnit * 22)
        spacing: Kirigami.Units.smallSpacing

        Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            width: Kirigami.Units.iconSizes.huge
            height: width
            radius: width / 2
            color: Qt.alpha(Kirigami.Theme.highlightColor, 0.14)

            Kirigami.Icon {
                anchors.centerIn: parent
                width: Kirigami.Units.iconSizes.medium
                height: width
                source: status.iconName
                color: Kirigami.Theme.highlightColor
                isMask: true
            }
        }

        Item {
            width: 1
            height: Kirigami.Units.smallSpacing
        }

        Text {
            width: parent.width
            visible: text !== ""
            text: status.title
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap
            color: Kirigami.Theme.textColor
            font.pointSize: Kirigami.Theme.defaultFont.pointSize * 1.1
            font.weight: Font.DemiBold
        }

        Text {
            width: parent.width
            visible: text !== ""
            text: status.text
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap
            color: Qt.alpha(Kirigami.Theme.textColor, 0.7)
            font.pointSize: Kirigami.Theme.defaultFont.pointSize * 0.95
        }

        Item {
            width: 1
            height: Kirigami.Units.smallSpacing
            visible: status.actionText !== ""
        }

        Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            visible: status.actionText !== ""
            width: actionLabel.implicitWidth + Kirigami.Units.gridUnit * 1.5
            height: Kirigami.Units.gridUnit * 1.8
            radius: height / 2
            color: actionTap.pressed ? Qt.darker(Kirigami.Theme.highlightColor, 1.15) : Kirigami.Theme.highlightColor
            scale: actionTap.pressed ? 0.95 : 1
            Behavior on scale {
                NumberAnimation { duration: Kirigami.Units.shortDuration * status.animationScale; easing.type: Easing.OutCubic }
            }

            Text {
                id: actionLabel
                anchors.centerIn: parent
                text: status.actionText
                color: Kirigami.Theme.highlightedTextColor
                font.weight: Font.DemiBold
            }

            TapHandler {
                id: actionTap
                onTapped: status.actionTriggered()
            }
        }
    }
}
