import QtQuick
import org.kde.kirigami as Kirigami

Rectangle {
    id: button

    property string iconName
    property real idleOpacity: 0.07
    property real animationScale: 1

    signal clicked()

    radius: width / 2
    color: Qt.alpha(Kirigami.Theme.textColor, tap.pressed ? 0.16 : idleOpacity)
    scale: tap.pressed ? 0.9 : 1

    Behavior on scale {
        NumberAnimation {
            duration: Kirigami.Units.shortDuration * button.animationScale
            easing.type: Easing.OutCubic
        }
    }

    Kirigami.Icon {
        anchors.centerIn: parent
        width: Math.round(button.width * 0.5)
        height: width
        source: button.iconName
        color: Kirigami.Theme.textColor
        isMask: true
    }

    TapHandler {
        id: tap
        onTapped: button.clicked()
    }
}
