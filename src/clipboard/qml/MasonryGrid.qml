pragma ComponentBehavior: Bound

import QtQuick
import org.kde.kirigami as Kirigami
import org.devl0rd.kboard.clipboard

Flickable {
    id: grid

    property string filter: "all"
    property int columns: 2
    property int currentIndex: -1
    property bool hidingUnpinned: false
    property int shownCount: 0
    property bool ready: false
    readonly property real spacing: Kirigami.Units.largeSpacing

    signal activated(int row)

    function matches(card) {
        switch (filter) {
        case "pinned": return card.pinned
        case "images": return card.kind === "image"
        case "links": return card.kind === "url"
        default: return true
        }
    }

    function relayout() {
        const columnWidth = Math.floor((width - spacing * (columns - 1)) / columns)
        const heights = new Array(columns).fill(0)
        let shown = 0
        for (let i = 0; i < repeater.count; ++i) {
            const card = repeater.itemAt(i) as ClipboardCard
            if (!card)
                continue
            card.width = columnWidth
            card.shown = matches(card) && !(hidingUnpinned && !card.pinned)
            if (card.shown) {
                const column = heights.indexOf(Math.min(...heights))
                card.targetX = column * (columnWidth + spacing)
                card.targetY = heights[column]
                heights[column] += card.implicitHeight + spacing
                ++shown
            }
        }
        content.height = Math.max(0, Math.max(...heights) - spacing)
        shownCount = shown
        currentIndex = Math.min(currentIndex, repeater.count - 1)
    }

    function scheduleLayout() {
        Qt.callLater(relayout)
    }

    function cardAt(row: int): ClipboardCard {
        return row >= 0 ? repeater.itemAt(row) as ClipboardCard : null
    }

    function firstShown() {
        for (let i = 0; i < repeater.count; ++i) {
            if (cardAt(i) && cardAt(i).shown)
                return i
        }
        return -1
    }

    function directionalScore(card, from, dx, dy) {
        const ox = card.targetX + card.width / 2 - (from.targetX + from.width / 2)
        const oy = card.targetY + card.height / 2 - (from.targetY + from.height / 2)
        const along = dx !== 0 ? ox * dx : oy * dy
        const across = dx !== 0 ? Math.abs(oy) : Math.abs(ox)
        if (along <= (dx !== 0 ? from.width / 2 : 1))
            return Infinity
        return along + across * (dx !== 0 ? 2 : 4)
    }

    function moveFocus(dx, dy) {
        const from = cardAt(currentIndex)
        if (!from || !from.shown) {
            currentIndex = firstShown()
            ensureVisible()
            return
        }
        let best = -1
        let bestScore = Infinity
        for (let i = 0; i < repeater.count; ++i) {
            const card = cardAt(i)
            const score = card && card.shown && i !== currentIndex ? directionalScore(card, from, dx, dy) : Infinity
            if (score < bestScore) {
                bestScore = score
                best = i
            }
        }
        if (best >= 0)
            currentIndex = best
        ensureVisible()
    }

    function ensureVisible() {
        const card = cardAt(currentIndex)
        if (!card)
            return
        const top = card.targetY + content.y
        if (top < contentY)
            contentY = Math.max(0, top - spacing)
        else if (top + card.height > contentY + height)
            contentY = Math.min(contentHeight - height, top + card.height - height + spacing)
    }

    function dismissCurrent() {
        const card = cardAt(currentIndex)
        if (card)
            card.dismiss(-1)
    }

    contentWidth: width
    contentHeight: content.height + spacing * 2
    flickableDirection: Flickable.VerticalFlick
    boundsBehavior: Flickable.DragOverBounds
    clip: true
    pressDelay: 0

    onFilterChanged: {
        contentY = 0
        scheduleLayout()
    }
    onColumnsChanged: scheduleLayout()
    onWidthChanged: scheduleLayout()
    onHidingUnpinnedChanged: scheduleLayout()
    Component.onCompleted: {
        relayout()
        Qt.callLater(() => { grid.ready = true })
    }

    Behavior on contentY { enabled: !grid.moving; NumberAnimation { duration: ClipboardStyle.duration(220); easing.type: Easing.OutCubic } }

    Connections {
        target: ClipboardHistory
        function onRowsMoved() { grid.scheduleLayout() }
        function onModelReset() { grid.scheduleLayout() }
        function onDataChanged() { grid.scheduleLayout() }
    }

    Item {
        id: content
        y: grid.spacing
        width: grid.width

        Behavior on height { enabled: grid.ready; NumberAnimation { duration: ClipboardStyle.duration(260); easing.type: Easing.OutCubic } }

        Repeater {
            id: repeater
            model: ClipboardHistory
            onItemAdded: grid.scheduleLayout()
            onItemRemoved: grid.scheduleLayout()

            delegate: ClipboardCard {
                id: delegateCard
                focused: grid.currentIndex === delegateCard.index
                animateLayout: grid.ready
                enterDelay: grid.ready ? 0 : Math.min(delegateCard.index, 12) * 28
                maximumImageHeight: Math.max(Kirigami.Units.gridUnit * 5, Math.min(Kirigami.Units.gridUnit * 10, grid.height * 0.55))
                onImplicitHeightChanged: grid.scheduleLayout()
                onActivated: grid.activated(delegateCard.index)
                onPinToggled: ClipboardHistory.pin(delegateCard.index, !delegateCard.pinned)
                onRemoveRequested: ClipboardHistory.remove(delegateCard.index)
            }
        }
    }
}
