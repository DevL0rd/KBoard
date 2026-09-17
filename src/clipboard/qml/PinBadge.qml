import QtQuick
import org.kde.kirigami as Kirigami

Rectangle {
    id: badge

    property bool pinned: false

    width: Kirigami.Units.gridUnit * 1.25
    height: width
    radius: width / 2
    color: ClipboardStyle.accentColor
    border.width: 2
    border.color: Kirigami.Theme.backgroundColor
    scale: pinned ? 1 : 0
    rotation: pinned ? 0 : -75
    visible: scale > 0

    Behavior on scale { NumberAnimation { duration: ClipboardStyle.duration(380); easing.type: Easing.OutBack; easing.overshoot: 2.8 } }
    Behavior on rotation { NumberAnimation { duration: ClipboardStyle.duration(380); easing.type: Easing.OutCubic } }

    Kirigami.Icon {
        anchors.centerIn: parent
        width: Math.round(parent.width * 0.58)
        height: width
        source: "pin"
        color: Kirigami.Theme.highlightedTextColor
        isMask: true
    }
}
