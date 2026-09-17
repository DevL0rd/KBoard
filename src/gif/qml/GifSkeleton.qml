pragma ComponentBehavior: Bound

import QtQuick
import org.kde.kirigami as Kirigami

Item {
    id: skeleton

    property int columns: 3
    property real spacing: Kirigami.Units.smallSpacing
    property real animationScale: 1

    readonly property var ratios: [1.0, 0.72, 1.28, 0.86, 1.12, 0.64]
    readonly property real tileWidth: (width - spacing * (columns - 1)) / columns

    function ratioAt(column, row) {
        return ratios[(column * 2 + row) % ratios.length]
    }

    function offsetAt(column, row) {
        let total = 0
        for (let r = 0; r < row; ++r) {
            total += tileWidth * ratioAt(column, r) + spacing
        }
        return total
    }

    clip: true
    visible: opacity > 0

    Behavior on opacity {
        NumberAnimation { duration: Kirigami.Units.longDuration * skeleton.animationScale; easing.type: Easing.OutCubic }
    }

    Repeater {
        model: skeleton.columns * 3

        delegate: GifTile {
            required property int index
            readonly property int column: index % skeleton.columns
            readonly property int row: Math.floor(index / skeleton.columns)

            x: column * (skeleton.tileWidth + skeleton.spacing)
            y: skeleton.offsetAt(column, row)
            width: skeleton.tileWidth
            height: skeleton.tileWidth * skeleton.ratioAt(column, row)
            inView: skeleton.visible
            busy: true
            animationScale: skeleton.animationScale
        }
    }
}
