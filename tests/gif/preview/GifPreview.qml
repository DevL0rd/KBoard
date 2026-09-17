import QtQuick
import QtQuick.Window
import org.kde.kirigami as Kirigami
import org.devl0rd.kboard.gif

Window {
    id: window

    readonly property var args: Qt.application.arguments
    readonly property string fixtureDir: args[args.length - 3]
    readonly property string outputPath: args[args.length - 2]
    readonly property string scene: args[args.length - 1]

    width: 1100
    height: 470
    visible: true
    Rectangle {
        anchors.fill: parent
        color: Kirigami.Theme.backgroundColor
    }

    GifPanel {
        id: panel
        anchors.fill: parent
    }

    Component.onCompleted: {
        if (scene !== "unconfigured") {
            GifStore.loadFixtures(fixtureDir)
        }
        if (scene === "trending") {
            GifStore.toggleFavorite(GifStore.trending.get(1).itemId)
            GifStore.toggleFavorite(GifStore.trending.get(3).itemId)
        }
        if (scene === "toast") {
            GifStore.insert(GifStore.trending.get(2).itemId)
        }
        if (scene === "search") {
            panel.searchActive = true
            panel.appendSearch("hello")
        } else if (scene === "empty") {
            panel.selectSection("favorites", "")
        } else {
            panel.selectSection("trending", "")
        }
        grabTimer.start()
    }

    Timer {
        id: grabTimer
        interval: 2500
        onTriggered: window.contentItem.grabToImage(result => {
            result.saveToFile(window.outputPath)
            Qt.quit()
        })
    }
}
