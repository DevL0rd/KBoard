pragma ComponentBehavior: Bound

import QtQuick
import org.kde.kirigami as Kirigami
import org.devl0rd.kboard.config

Item {
    id: closeUp

    property string centerKey: "g"
    property real rowsVisible: 3.3
    property real topFraction: 0.3
    property alias keyboard: keyboard
    property color accent: Settings.accentMode === 1 ? Settings.accentColor : Kirigami.Theme.highlightColor

    Wallpaper {
        id: wallpaper
        anchors.fill: parent
        radius: Kirigami.Units.cornerRadius * 2
        accent: closeUp.accent
    }

    Item {
        anchors.fill: parent
        clip: true

        MiniKeyboard {
            id: keyboard
            readonly property real rowHeight: closeUp.height / closeUp.rowsVisible
            readonly property real anchorX: {
                const key = keyData(closeUp.centerKey);
                return key ? (key.x + key.w / 2) * width / 10 : width / 2;
            }
            width: rowHeight * 10 / 1.05
            height: rowHeight * model.height
            x: Math.max(closeUp.width - width, Math.min(0, closeUp.width / 2 - anchorX))
            y: closeUp.height * closeUp.topFraction
            metric: rowHeight / 52
            backdrop: wallpaper
        }
    }

    Repeater {
        model: 2

        Rectangle {
            id: fade
            required property int index
            readonly property bool leading: index === 0
            x: fade.leading ? 0 : closeUp.width - width
            width: Math.min(closeUp.width * 0.2, Kirigami.Units.gridUnit * 2)
            height: closeUp.height
            gradient: Gradient {
                orientation: Gradient.Horizontal
                GradientStop {
                    position: 0
                    color: Qt.alpha(Kirigami.Theme.backgroundColor, fade.leading ? 0.85 : 0)
                }
                GradientStop {
                    position: 1
                    color: Qt.alpha(Kirigami.Theme.backgroundColor, fade.leading ? 0 : 0.85)
                }
            }
        }
    }

    Rectangle {
        anchors.bottom: parent.bottom
        width: parent.width
        height: Math.min(parent.height * 0.25, Kirigami.Units.gridUnit * 2)
        gradient: Gradient {
            GradientStop {
                position: 0
                color: Qt.alpha(Kirigami.Theme.backgroundColor, 0)
            }
            GradientStop {
                position: 1
                color: Qt.alpha(Kirigami.Theme.backgroundColor, 0.9)
            }
        }
    }

    Rectangle {
        anchors.fill: parent
        radius: Kirigami.Units.cornerRadius * 2
        color: "transparent"
        border.color: Qt.alpha(Kirigami.Theme.textColor, 0.2)
    }
}
