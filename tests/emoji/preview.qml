import QtQuick
import QtQuick.Window
import org.kde.kirigami as Kirigami
import org.devl0rd.kboard.emoji

Window {
    id: window

    readonly property var args: Qt.application.arguments
    readonly property string output: argument("--out", "")
    readonly property string scenario: argument("--scenario", "browse")
    readonly property string group: argument("--group", "")

    function argument(name, fallbackValue) {
        const index = args.indexOf(name)
        return index >= 0 && index + 1 < args.length ? args[index + 1] : fallbackValue
    }

    readonly property int wantedWidth: Number(argument("--width", "1280"))
    readonly property int wantedHeight: Number(argument("--height", "300"))

    width: wantedWidth
    height: wantedHeight
    minimumWidth: wantedWidth
    maximumWidth: wantedWidth
    minimumHeight: wantedHeight
    maximumHeight: wantedHeight
    visible: true
    color: Kirigami.Theme.backgroundColor

    Rectangle {
        id: surface
        anchors.fill: parent
        color: Kirigami.Theme.backgroundColor

        EmojiPanel {
            id: panel
            anchors.fill: parent
            onSearchRequested: searchActive = true
        }
    }

    Timer {
        interval: 600
        running: true
        onTriggered: {
            if (window.group !== "") {
                panel.openGroup(window.group)
            }
            if (window.scenario === "search") {
                panel.searchActive = true
                panel.searchText = window.argument("--query", "heart")
            }
            captureTimer.start()
        }
    }

    Timer {
        id: popupTimer
        interval: 50
        onTriggered: {
            const cell = window.findCell(panel, window.argument("--emoji", "👍"))
            if (cell) {
                panel.openPopup(cell)
            } else {
                console.warn("preview: cell not found")
            }
        }
    }

    function findCell(item, text) {
        for (let i = 0; i < item.children.length; ++i) {
            const child = item.children[i]
            if (child.base !== undefined && child.tones !== undefined && child.base === text && child.visible) {
                return child
            }
            const found = window.findCell(child, text)
            if (found) {
                return found
            }
        }
        return null
    }

    Timer {
        id: captureTimer
        property bool popupRequested: false
        interval: 700
        onTriggered: {
            if (window.scenario === "popup" && !popupRequested) {
                popupRequested = true
                popupTimer.start()
                captureTimer.start()
                return
            }
            surface.grabToImage(result => {
                if (window.output !== "") {
                    result.saveToFile(window.output)
                }
                Qt.quit()
            })
        }
    }
}
