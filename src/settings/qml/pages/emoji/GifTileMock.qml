pragma ComponentBehavior: Bound

import QtQuick
import org.kde.kirigami as Kirigami

Rectangle {
    id: tile

    property int seed: 0
    property bool animated: true
    property real phase: 0
    property real pressed: 0

    readonly property var hues: [0.58, 0.08, 0.83, 0.33, 0.95, 0.5]

    radius: Kirigami.Units.cornerRadius
    clip: true
    scale: 1 - 0.08 * pressed
    gradient: Gradient {
        orientation: Gradient.Horizontal
        GradientStop {
            position: 0
            color: Qt.hsla(tile.hues[tile.seed % 6], 0.55, 0.5, 1)
        }
        GradientStop {
            position: 1
            color: Qt.hsla((tile.hues[tile.seed % 6] + 0.12) % 1, 0.6, 0.35, 1)
        }
    }

    Rectangle {
        readonly property real shift: tile.animated ? (tile.phase * (1 + tile.seed % 3)) % 1 : 0.3
        width: parent.width * 0.5
        height: width
        radius: width / 2
        x: -width / 2 + (parent.width + width) * shift
        y: parent.height * (0.2 + 0.1 * Math.sin(shift * Math.PI * 2 + tile.seed))
        color: Qt.alpha("white", 0.35)
    }

    Kirigami.Icon {
        anchors.centerIn: parent
        visible: !tile.animated
        width: Math.min(parent.width, parent.height) * 0.4
        height: width
        source: "media-playback-start"
        color: "white"
        isMask: true
    }
}
