pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.devl0rd.kboard.config
import org.devl0rd.kboard.gif

Flickable {
    id: grid

    required property GifModel gifModel
    property int columns: 3
    property real spacing: Kirigami.Units.smallSpacing
    property bool remote: false
    property bool active: true
    property bool hidden: false
    property real animationScale: 1

    readonly property bool showFooter: GifStore.loadingMore && remote
    readonly property real viewTop: contentY - height * 0.5
    readonly property real viewBottom: contentY + height * 1.5

    clip: true
    contentWidth: width - leftMargin - rightMargin
    contentHeight: gifModel.contentHeight + (showFooter ? Kirigami.Units.gridUnit * 3 : 0)
    boundsBehavior: Flickable.DragOverBounds
    flickableDirection: Flickable.VerticalFlick
    pixelAligned: true

    onContentYChanged: maybeLoadMore()
    onContentHeightChanged: maybeLoadMore()

    function maybeLoadMore() {
        if (remote && gifModel.hasMore && gifModel.count > 0 && contentY + height > contentHeight - height) {
            GifStore.loadMore()
        }
    }

    Binding {
        target: grid.gifModel
        property: "columns"
        value: grid.columns
    }
    Binding {
        target: grid.gifModel
        property: "layoutWidth"
        value: grid.contentWidth
    }
    Binding {
        target: grid.gifModel
        property: "spacing"
        value: grid.spacing
    }

    Item {
        width: grid.contentWidth
        height: grid.gifModel.contentHeight
        opacity: grid.hidden ? 0 : 1

        Behavior on opacity {
            NumberAnimation { duration: Kirigami.Units.longDuration * grid.animationScale; easing.type: Easing.OutCubic }
        }

        Repeater {
            model: grid.gifModel

            delegate: GifTile {
                required property var model

                itemId: model.itemId
                previewUrl: model.previewUrl
                blurPreview: model.blurPreview
                favorite: model.favorite
                sticker: model.type === "sticker"
                x: model.tileX
                y: model.tileY
                width: model.tileWidth
                height: model.tileHeight
                inView: y + height > grid.viewTop && y < grid.viewBottom
                playing: Settings.gifAutoplay && grid.active && y + height > grid.contentY && y < grid.contentY + grid.height
                inserting: GifStore.insertingId === itemId
                busy: GifStore.inserting && !inserting
                animationScale: grid.animationScale
                onActivated: GifStore.insert(itemId)
                onFavoriteToggled: GifStore.toggleFavorite(itemId)
            }
        }
    }

    QQC2.BusyIndicator {
        y: grid.gifModel.contentHeight + Kirigami.Units.gridUnit * 0.5
        x: (grid.contentWidth - width) / 2
        width: Kirigami.Units.iconSizes.medium
        height: width
        visible: grid.showFooter
        running: visible
    }
}
