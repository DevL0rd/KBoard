import QtQuick
import org.kde.kirigami as Kirigami

Item {
    id: chip

    property string text
    property string iconName
    property bool selected: false
    property real animationScale: 1

    signal clicked()

    implicitHeight: Kirigami.Units.gridUnit * 1.6
    implicitWidth: content.implicitWidth + Kirigami.Units.largeSpacing * 1.5

    Rectangle {
        id: background
        anchors.fill: parent
        radius: height / 2
        color: chip.selected ? "transparent" : Qt.alpha(Kirigami.Theme.textColor, tap.pressed ? 0.16 : 0.07)
        border.width: 1
        border.color: chip.selected ? "transparent" : Qt.alpha(Kirigami.Theme.textColor, 0.08)
        Behavior on color {
            ColorAnimation { duration: Kirigami.Units.shortDuration * chip.animationScale; easing.type: Easing.OutCubic }
        }
    }

    Row {
        id: content
        anchors.centerIn: parent
        spacing: Kirigami.Units.smallSpacing

        Kirigami.Icon {
            visible: chip.iconName !== ""
            source: chip.iconName
            width: Kirigami.Units.iconSizes.small
            height: width
            anchors.verticalCenter: parent.verticalCenter
            color: label.color
            isMask: true
        }

        Text {
            id: label
            text: chip.text
            anchors.verticalCenter: parent.verticalCenter
            color: chip.selected ? Kirigami.Theme.highlightedTextColor : Kirigami.Theme.textColor
            font.pointSize: Kirigami.Theme.defaultFont.pointSize * 0.95
            font.weight: chip.selected ? Font.DemiBold : Font.Normal
            Behavior on color {
                ColorAnimation { duration: Kirigami.Units.shortDuration * chip.animationScale; easing.type: Easing.OutCubic }
            }
        }
    }

    scale: tap.pressed ? 0.94 : 1
    Behavior on scale {
        NumberAnimation { duration: Kirigami.Units.shortDuration * chip.animationScale; easing.type: Easing.OutCubic }
    }

    TapHandler {
        id: tap
        onTapped: chip.clicked()
    }
}
