import QtQuick
import org.kde.kirigami as Kirigami

Rectangle {
    id: banner

    property bool active: false

    readonly property real fullHeight: row.implicitHeight + Kirigami.Units.smallSpacing * 2

    implicitWidth: row.implicitWidth + Kirigami.Units.largeSpacing * 3
    implicitHeight: active ? fullHeight : 0
    radius: fullHeight / 2
    color: Qt.alpha(Kirigami.Theme.neutralTextColor, 0.16)
    border.width: 1
    border.color: Qt.alpha(Kirigami.Theme.neutralTextColor, 0.35)
    opacity: active ? 1 : 0
    scale: active ? 1 : 0.9
    visible: implicitHeight > 0.5
    clip: true

    Behavior on implicitHeight { NumberAnimation { duration: ClipboardStyle.duration(240); easing.type: Easing.OutCubic } }
    Behavior on opacity { NumberAnimation { duration: ClipboardStyle.duration(200) } }
    Behavior on scale { NumberAnimation { duration: ClipboardStyle.duration(240); easing.type: Easing.OutBack } }

    Row {
        id: row
        anchors.centerIn: parent
        spacing: Kirigami.Units.smallSpacing

        Kirigami.Icon {
            width: Kirigami.Units.iconSizes.small
            height: width
            anchors.verticalCenter: parent.verticalCenter
            source: "media-playback-pause"
            color: Kirigami.Theme.neutralTextColor
            isMask: true
        }

        Text {
            anchors.verticalCenter: parent.verticalCenter
            text: i18n("New copies are not being saved")
            color: Kirigami.Theme.textColor
            font: Kirigami.Theme.smallFont
        }
    }
}
