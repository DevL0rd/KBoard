import QtQuick
import org.kde.kirigami as Kirigami
import org.devl0rd.kboard.config
import org.devl0rd.kboard.platform
import org.devl0rd.kboard.emoji

Item {
    id: panel

    property string searchText: ""
    property bool searchActive: false
    property bool showBackspace: true

    readonly property real animScale: Settings.animationsEnabled ? 1 / Math.max(0.05, Settings.animationSpeed) : 0
    readonly property string currentGroup: tabBar.currentId
    readonly property real sideMargin: Kirigami.Units.smallSpacing * 2
    readonly property real contentWidth: width - sideMargin * 2
    readonly property real barHeight: Math.round(Math.max(Kirigami.Units.gridUnit * 1.7, Math.min(height * 0.13, Kirigami.Units.gridUnit * 2.4)))
    readonly property real gridHeight: Math.max(1, height - barHeight * (searchActive ? 1 : 2) - Kirigami.Units.smallSpacing * 3)
    readonly property real targetCell: Math.max(38, Math.min(66, Math.min(contentWidth / 8, gridHeight / 3.2)))
    readonly property int columns: Math.max(4, Math.floor(contentWidth / targetCell))
    readonly property real cellWidth: contentWidth / columns
    readonly property real cellHeight: Math.round(Math.min(cellWidth, targetCell * 1.04))

    signal searchRequested()
    signal searchClosed()
    signal closeRequested()
    signal emojiCommitted(string text)

    function appendSearch(text) {
        searchText += text
    }

    function backspaceSearch() {
        const chars = Array.from(searchText)
        chars.pop()
        searchText = chars.join("")
    }

    function clearSearch() {
        searchText = ""
    }

    function closeSearch() {
        searchActive = false
        searchText = ""
        searchClosed()
    }

    function refresh() {
        gridModel.refresh()
    }

    function openGroup(id) {
        if (searchActive) {
            closeSearch()
        }
        grid.jumpTo(id)
    }

    function openPopup(cell) {
        popup.showFor(cell, panel, sideMargin)
    }

    function commit(text, kaomoji) {
        InputContext.commit(text)
        if (!kaomoji) {
            EmojiStore.recordUse(text)
        }
        emojiCommitted(text)
    }

    function applyPick(option, cell) {
        if (option.kind === "favorite") {
            EmojiStore.toggleFavorite(cell.base)
            return
        }
        if (cell.tones) {
            EmojiStore.setPreferredTone(option.text, EmojiStore.skinToneOf(option.text))
        }
        commit(option.text, cell.kaomoji)
    }

    onSearchActiveChanged: {
        popup.dismiss()
        grid.positionViewAtBeginning()
    }
    onVisibleChanged: {
        if (visible && gridModel.stale && !searchActive) {
            gridModel.refresh()
        }
    }

    EmojiGridModel {
        id: gridModel
        columns: panel.columns
        kaomojiColumns: Math.max(2, Math.min(5, Math.floor(panel.contentWidth / (Kirigami.Units.gridUnit * 8))))
        searchText: panel.searchActive ? panel.searchText : ""
    }

    PanelTopBar {
        id: topBar
        anchors.top: parent.top
        anchors.topMargin: Kirigami.Units.smallSpacing
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: panel.sideMargin
        anchors.rightMargin: panel.sideMargin
        height: panel.barHeight
        searchText: panel.searchText
        searchActive: panel.searchActive
        showBackspace: panel.showBackspace
        animScale: panel.animScale
        onBackRequested: panel.searchActive ? panel.closeSearch() : panel.closeRequested()
        onSearchRequested: panel.searchRequested()
        onClearRequested: panel.clearSearch()
        onBackspaceRequested: panel.searchActive ? panel.backspaceSearch() : InputContext.backspace()
    }

    EmojiTabBar {
        id: tabBar
        anchors.top: topBar.bottom
        anchors.topMargin: Kirigami.Units.smallSpacing
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: panel.sideMargin
        anchors.rightMargin: panel.sideMargin
        height: panel.searchActive ? 0 : panel.barHeight
        opacity: panel.searchActive ? 0 : 1
        visible: height > 0
        groups: EmojiStore.groups
        currentId: grid.activeSection
        animScale: panel.animScale
        onActivated: id => panel.openGroup(id)
        Behavior on height { NumberAnimation { duration: Math.round(220 * panel.animScale); easing.type: Easing.OutCubic } }
        Behavior on opacity { NumberAnimation { duration: Math.round(180 * panel.animScale) } }
    }

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: grid.top
        height: 1
        color: Qt.alpha(Kirigami.Theme.textColor, 0.08)
        opacity: grid.contentY - grid.originY > 2 ? 1 : 0
        Behavior on opacity { NumberAnimation { duration: Math.round(150 * panel.animScale) } }
    }

    EmojiGrid {
        id: grid
        anchors.top: tabBar.bottom
        anchors.topMargin: Kirigami.Units.smallSpacing
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.leftMargin: panel.sideMargin
        anchors.rightMargin: panel.sideMargin
        visible: EmojiStore.ready
        gridModel: gridModel
        cellWidth: panel.cellWidth
        cellHeight: panel.cellHeight
        glyphSize: Math.round(Math.min(panel.cellWidth, panel.cellHeight) * 0.64)
        kaomojiColumns: gridModel.kaomojiColumns
        animScale: panel.animScale
        searchActive: panel.searchActive
        onMovementStarted: popup.dismiss()
        onCellActivated: cell => panel.commit(cell.text, cell.kaomoji)
        onCellHeld: cell => panel.openPopup(cell)
        onCellDragged: (cell, x, y) => popup.track(cell, x, y)
        onCellDropped: cell => popup.release(cell)
    }

    Kirigami.PlaceholderMessage {
        anchors.centerIn: grid
        width: grid.width - Kirigami.Units.gridUnit * 2
        visible: !EmojiStore.ready
        icon.name: "data-error"
        text: "Emoji are unavailable"
        explanation: EmojiStore.errorString
    }

    MouseArea {
        anchors.fill: parent
        visible: popup.open && !(popup.source && popup.source.holding)
        z: 9
        onPressed: popup.dismiss()
    }

    SkinTonePopup {
        id: popup
        z: 10
        optionSize: Math.round(Math.max(40, Math.min(panel.cellHeight * 1.05, 60)))
        kaomojiWidth: Math.round(Math.min(panel.width * 0.6, Kirigami.Units.gridUnit * 12))
        animScale: panel.animScale
        onPicked: (option, cell) => panel.applyPick(option, cell)
    }
}
