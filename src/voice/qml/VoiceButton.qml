import QtQuick
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Rectangle {
    id: button

    property string text
    property string iconName
    property bool primary: false
    property bool round: false
    signal clicked()

    readonly property color foreground: primary ? VoiceStyle.accentText : VoiceStyle.ink

    implicitHeight: Kirigami.Units.gridUnit * 2.6
    implicitWidth: round ? implicitHeight : Math.max(implicitHeight * 2.4, row.implicitWidth + Kirigami.Units.largeSpacing * 3)
    radius: height / 2
    opacity: enabled ? 1 : 0.45
    color: {
        if (primary)
            return area.pressed ? Qt.darker(VoiceStyle.accent, 1.15) : VoiceStyle.accent
        if (round && !area.containsMouse && !area.pressed)
            return "transparent"
        return Qt.alpha(VoiceStyle.ink, area.pressed ? 0.18 : (area.containsMouse ? 0.12 : 0.08))
    }
    border.width: primary || round ? 0 : 1
    border.color: Qt.alpha(VoiceStyle.ink, 0.1)
    scale: area.pressed ? 0.96 : 1

    Accessible.role: Accessible.Button
    Accessible.name: text
    Accessible.onPressAction: clicked()

    Behavior on scale { NumberAnimation { duration: VoiceStyle.duration(120); easing.type: Easing.OutCubic } }
    Behavior on color { ColorAnimation { duration: VoiceStyle.duration(150) } }

    RowLayout {
        id: row
        anchors.centerIn: parent
        spacing: Kirigami.Units.smallSpacing

        Kirigami.Icon {
            source: button.iconName
            implicitWidth: button.round ? Kirigami.Units.iconSizes.smallMedium : Kirigami.Units.iconSizes.small
            implicitHeight: implicitWidth
            isMask: true
            color: button.foreground
        }

        Text {
            visible: !button.round
            text: button.text
            color: button.foreground
            font.pixelSize: Kirigami.Theme.defaultFont.pixelSize * 1.05
            font.weight: Font.DemiBold
        }
    }

    MouseArea {
        id: area
        anchors.fill: parent
        hoverEnabled: true
        onClicked: button.clicked()
    }
}
