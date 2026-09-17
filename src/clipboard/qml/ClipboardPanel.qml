import QtQuick
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.devl0rd.kboard.clipboard

Item {
    id: panel

    property alias filter: grid.filter
    property int columns: Math.max(2, Math.floor(width / (Kirigami.Units.gridUnit * 22)))
    property alias currentIndex: grid.currentIndex
    property bool closeOnPaste: false
    readonly property alias shownCount: grid.shownCount
    readonly property alias clearArmed: header.clearArmed

    signal closeRequested()
    signal pasted(int row)

    function activate(row) {
        if (!ClipboardHistory.paste(row))
            return
        pasted(row)
        if (closeOnPaste)
            closeRequested()
    }

    function moveFocus(dx, dy) {
        grid.moveFocus(dx, dy)
    }

    function activateCurrent() {
        if (currentIndex >= 0)
            activate(currentIndex)
    }

    function togglePinCurrent() {
        if (currentIndex >= 0)
            ClipboardHistory.pin(currentIndex, !ClipboardHistory.get(currentIndex).pinned)
    }

    function removeCurrent() {
        grid.dismissCurrent()
    }

    function requestClear() {
        header.requestClear()
    }

    SequentialAnimation {
        id: clearing
        PropertyAction { target: grid; property: "hidingUnpinned"; value: true }
        PauseAnimation { duration: ClipboardStyle.duration(260) }
        ScriptAction { script: ClipboardHistory.clear() }
        PropertyAction { target: grid; property: "hidingUnpinned"; value: false }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        ClipboardHeader {
            id: header
            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.smallSpacing
            Layout.bottomMargin: 0
            filter: grid.filter
            clearEnabled: ClipboardHistory.count > ClipboardHistory.pinnedCount && !clearing.running
            onBackRequested: panel.closeRequested()
            onFilterRequested: key => grid.filter = key
            onClearConfirmed: clearing.start()
        }

        Kirigami.InlineMessage {
            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.smallSpacing
            type: Kirigami.MessageType.Error
            text: ClipboardHistory.errorString
            visible: text.length > 0
        }

        PausedBanner {
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: active ? Kirigami.Units.smallSpacing : 0
            active: ClipboardHistory.paused && ClipboardHistory.enabled
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            MasonryGrid {
                id: grid
                anchors.fill: parent
                anchors.leftMargin: Kirigami.Units.largeSpacing
                anchors.rightMargin: Kirigami.Units.largeSpacing
                columns: panel.columns
                visible: ClipboardHistory.enabled
                onActivated: row => panel.activate(row)
            }

            ClipboardEmptyState {
                anchors.centerIn: parent
                width: parent.width - Kirigami.Units.gridUnit * 4
                filter: grid.filter
                visible: !ClipboardHistory.enabled || (grid.shownCount === 0 && !clearing.running)
            }
        }
    }
}
