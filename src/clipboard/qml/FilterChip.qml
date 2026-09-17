import QtQuick
import org.kde.kirigami as Kirigami

Item {
    id: chip

    property string label
    property string iconName
    property bool checked: false

    signal clicked()

    implicitWidth: content.implicitWidth + Kirigami.Units.largeSpacing * 2.5
    implicitHeight: Kirigami.Units.gridUnit * 1.7
    width: implicitWidth
    height: implicitHeight

    Rectangle {
        anchors.fill: parent
        radius: height / 2
        color: chip.checked ? ClipboardStyle.accentColor
            : tap.pressed ? Qt.alpha(Kirigami.Theme.textColor, 0.16)
            : Qt.alpha(Kirigami.Theme.textColor, hover.hovered ? 0.1 : 0.06)
        border.width: chip.checked ? 0 : 1
        border.color: Qt.alpha(Kirigami.Theme.textColor, 0.1)
        scale: tap.pressed ? 0.94 : 1
        Behavior on color { ColorAnimation { duration: ClipboardStyle.duration(180) } }
        Behavior on scale { NumberAnimation { duration: ClipboardStyle.duration(140); easing.type: Easing.OutCubic } }
    }

    Row {
        id: content
        anchors.centerIn: parent
        spacing: chip.checked ? Kirigami.Units.smallSpacing : 0

        Kirigami.Icon {
            width: chip.checked ? Kirigami.Units.iconSizes.small : 0
            height: Kirigami.Units.iconSizes.small
            anchors.verticalCenter: parent.verticalCenter
            source: chip.iconName
            color: Kirigami.Theme.highlightedTextColor
            isMask: true
            opacity: chip.checked ? 1 : 0
            Behavior on width { NumberAnimation { duration: ClipboardStyle.duration(180); easing.type: Easing.OutCubic } }
            Behavior on opacity { NumberAnimation { duration: ClipboardStyle.duration(180) } }
        }

        Text {
            anchors.verticalCenter: parent.verticalCenter
            text: chip.label
            color: chip.checked ? Kirigami.Theme.highlightedTextColor : Kirigami.Theme.textColor
            font.pointSize: Kirigami.Theme.defaultFont.pointSize
            font.weight: chip.checked ? Font.DemiBold : Font.Normal
        }
    }

    HoverHandler { id: hover }

    TapHandler {
        id: tap
        onTapped: chip.clicked()
    }
}
