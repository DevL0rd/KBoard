pragma ComponentBehavior: Bound

import QtQuick
import org.kde.kirigami as Kirigami

Rectangle {
    id: wallpaper

    property color accent: Kirigami.Theme.highlightColor
    property real drift: 0

    clip: true
    gradient: Gradient {
        orientation: Gradient.Vertical
        GradientStop {
            position: 0
            color: Qt.tint(Kirigami.Theme.backgroundColor, Qt.alpha(wallpaper.accent, 0.35))
        }
        GradientStop {
            position: 1
            color: Qt.tint(Kirigami.Theme.backgroundColor, Qt.alpha(Kirigami.Theme.positiveTextColor, 0.2))
        }
    }

    Repeater {
        model: [
            { x: 0.12, y: 0.72, size: 0.55, tint: "accent" },
            { x: 0.78, y: 0.82, size: 0.5, tint: "positive" },
            { x: 0.5, y: 0.3, size: 0.35, tint: "neutral" },
            { x: 0.95, y: 0.2, size: 0.3, tint: "accent" }
        ]

        Rectangle {
            required property var modelData
            required property int index
            readonly property real size: Math.max(wallpaper.width, wallpaper.height) * modelData.size
            x: wallpaper.width * modelData.x - size / 2 + Math.sin(wallpaper.drift * Math.PI * 2 + index) * size * 0.08
            y: wallpaper.height * modelData.y - size / 2
            width: size
            height: size
            radius: size / 2
            opacity: 0.55
            color: modelData.tint === "accent" ? wallpaper.accent : (modelData.tint === "positive" ? Kirigami.Theme.positiveTextColor : Kirigami.Theme.neutralTextColor)
        }
    }
}
