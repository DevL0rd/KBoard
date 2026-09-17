pragma ComponentBehavior: Bound

import QtQuick

Row {
    id: row

    required property var cells
    required property var view
    property bool kaomoji: false

    width: view.width
    height: kaomoji ? Math.round(view.cellHeight * 0.92) : view.cellHeight

    Repeater {
        model: row.cells

        EmojiCell {
            required property var modelData

            width: row.kaomoji ? row.width / row.view.kaomojiColumns : row.view.cellWidth
            height: row.height
            kaomoji: row.kaomoji
            text: modelData.text
            base: modelData.base
            name: modelData.name
            tones: modelData.tones
            favorite: modelData.favorite
            glyphSize: row.view.glyphSize
            animScale: row.view.animScale
            onActivated: cell => row.view.cellActivated(cell)
            onHeld: cell => row.view.cellHeld(cell)
            onDragged: (cell, x, y) => row.view.cellDragged(cell, x, y)
            onDropped: cell => row.view.cellDropped(cell)
        }
    }
}
