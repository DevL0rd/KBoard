pragma ComponentBehavior: Bound

import QtQuick
import org.kde.kirigami as Kirigami

Item {
    id: finger

    property real down: 1
    property real size: Kirigami.Units.gridUnit * 1.4
    property bool mouse: false

    width: size
    height: size

    Rectangle {
        visible: !finger.mouse
        anchors.centerIn: parent
        width: finger.size * (1.25 - 0.25 * finger.down)
        height: width
        radius: width / 2
        color: Qt.alpha(Kirigami.Theme.textColor, 0.12 + 0.18 * finger.down)
        border.width: 1.5
        border.color: Qt.alpha(Kirigami.Theme.textColor, 0.55)
    }

    Kirigami.Icon {
        visible: finger.mouse
        x: finger.size / 2
        y: finger.size / 2
        width: finger.size * 0.9
        height: width
        source: "input-mouse-symbolic"
        color: Kirigami.Theme.textColor
        isMask: true
        scale: 1 - 0.12 * finger.down
    }
}
