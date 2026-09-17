pragma ComponentBehavior: Bound

import QtQuick

Item {
    id: popup

    property var items: []
    property rect anchorRect
    property int touchId: -2
    property int keyIndex: -1
    property int selected: 0
    property int maxColumns: 8
    property real cellWidth: 40
    property real cellHeight: 48
    property real presence: 0

    readonly property bool open: keyIndex >= 0
    readonly property int columns: Math.max(1, Math.min(items.length, maxColumns))
    readonly property int rows: Math.max(1, Math.ceil(items.length / columns))
    readonly property real padding: Math.round(cellHeight * 0.12)
    readonly property bool mirrored: anchorRect.x + columns * cellWidth + padding * 2 > parent.width
    readonly property real anchorOffset: mirrored ? (columns - 1) * cellWidth + padding : padding

    function show(entries, rect, index, touch) {
        items = entries
        anchorRect = rect
        keyIndex = index
        touchId = touch
        selected = 0
        appear.restart()
    }

    function close() {
        keyIndex = -1
        touchId = -2
        presence = 0
    }

    function selectAt(px, py) {
        const localX = px - x - padding
        const localY = y + height - padding - py
        let column = Math.floor(localX / cellWidth)
        column = Math.max(0, Math.min(columns - 1, mirrored ? columns - 1 - column : column))
        const row = Math.max(0, Math.min(rows - 1, Math.floor(localY / cellHeight)))
        selected = Math.min(items.length - 1, row * columns + column)
    }

    function cellX(index) {
        const column = index % columns
        return padding + (mirrored ? columns - 1 - column : column) * cellWidth
    }

    function cellY(index) {
        return height - padding - (Math.floor(index / columns) + 1) * cellHeight
    }

    visible: open
    width: columns * cellWidth + padding * 2
    height: rows * cellHeight + padding * 2
    x: Math.max(0, Math.min(parent.width - width, anchorRect.x + anchorRect.width / 2 - cellWidth / 2 - anchorOffset))
    y: anchorRect.y - height + anchorRect.height * 0.08
    opacity: presence
    scale: 0.9 + 0.1 * presence
    transformOrigin: Item.Bottom

    SoftShadow {
        anchors.fill: background
        radius: background.radius
        spread: Math.round(height * 0.12)
        offsetY: Math.round(height * 0.04)
    }

    Rectangle {
        id: background

        anchors.fill: parent
        radius: Math.min(Theme.radius * 1.4, popup.cellHeight / 2)
        color: Theme.bubble
        border.width: 1
        border.color: Qt.alpha(Theme.text, Theme.dark ? 0.1 : 0.08)
    }

    Repeater {
        model: popup.open ? popup.items : []

        delegate: Item {
            id: cell

            required property int index
            required property var modelData
            property real entrance: 0

            x: popup.cellX(index)
            y: popup.cellY(index)
            width: popup.cellWidth
            height: popup.cellHeight
            opacity: entrance
            scale: 0.6 + 0.4 * entrance

            Rectangle {
                anchors.fill: parent
                anchors.margins: 2
                radius: Math.min(Theme.radius, height / 2)
                color: Theme.accent
                opacity: popup.selected === cell.index ? 1 : 0

                Behavior on opacity {
                    NumberAnimation {
                        duration: Theme.duration(90)
                    }
                }
            }

            Text {
                anchors.centerIn: parent
                text: cell.modelData.label
                color: popup.selected === cell.index ? Theme.accentText : Theme.keyText
                font.family: Theme.font.family
                font.pixelSize: Math.round(Math.min(popup.cellHeight * 0.44, popup.cellWidth * (text.length > 2 ? 0.2 : 0.55)) * Theme.labelScale)
                elide: Text.ElideRight
                width: Math.min(implicitWidth, popup.cellWidth - 8)
                horizontalAlignment: Text.AlignHCenter
            }

            SequentialAnimation {
                running: true

                PauseAnimation {
                    duration: Theme.duration(20) * cell.index
                }
                NumberAnimation {
                    target: cell
                    property: "entrance"
                    to: 1
                    duration: Theme.duration(150)
                    easing.type: Easing.OutBack
                }
            }
        }
    }

    NumberAnimation {
        id: appear

        target: popup
        property: "presence"
        from: 0
        to: 1
        duration: Theme.duration(110)
        easing.type: Easing.OutCubic
    }
}
