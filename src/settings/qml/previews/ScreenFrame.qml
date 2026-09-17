pragma ComponentBehavior: Bound

import QtQuick
import org.kde.kirigami as Kirigami

Item {
    id: frame

    property real aspect: 16 / 10
    property color accent: Kirigami.Theme.highlightColor
    property real drift: 0
    property bool showWindow: true
    property alias wallpaper: wallpaperItem
    property alias window: windowItem
    default property alias overlay: screen.data
    readonly property real screenWidth: screen.width
    readonly property real screenHeight: screen.height

    readonly property real fitWidth: Math.min(width, height * aspect)

    Rectangle {
        id: bezel
        width: frame.fitWidth
        height: width / frame.aspect
        anchors.centerIn: parent
        radius: Math.max(4, width * 0.03)
        color: Qt.darker(Kirigami.Theme.backgroundColor, 1.5)
        border.color: Qt.alpha(Kirigami.Theme.textColor, 0.3)
        border.width: 1

        Item {
            id: screen
            anchors.fill: parent
            anchors.margins: Math.max(3, bezel.width * 0.018)
            clip: true

            Wallpaper {
                id: wallpaperItem
                anchors.fill: parent
                radius: Math.max(2, bezel.radius * 0.6)
                accent: frame.accent
                drift: frame.drift
            }

            AppWindowMock {
                id: windowItem
                visible: frame.showWindow
                x: parent.width * 0.08
                y: parent.height * 0.07
                width: parent.width * 0.84
                height: parent.height * 0.5
            }
        }
    }
}
