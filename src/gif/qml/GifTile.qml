pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

Item {
    id: tile

    property string itemId
    property string title
    property url previewUrl
    property string blurPreview
    property bool favorite: false
    property bool sticker: false
    property bool inView: false
    property bool playing: false
    property bool inserting: false
    property bool busy: false
    property real radius: Kirigami.Units.cornerRadius * 2
    property real animationScale: 1

    signal activated()
    signal favoriteToggled()

    readonly property AnimatedImage image: imageLoader.item as AnimatedImage
    readonly property bool ready: image !== null && image.status === Image.Ready

    scale: tap.pressed ? 0.93 : 1
    Behavior on scale {
        NumberAnimation { duration: Kirigami.Units.shortDuration * tile.animationScale; easing.type: Easing.OutCubic }
    }

    Kirigami.ShadowedRectangle {
        id: base
        anchors.fill: parent
        radius: tile.radius
        visible: !tile.ready || tile.sticker
        color: Qt.alpha(Kirigami.Theme.textColor, tile.sticker ? 0.04 : 0.08)
    }

    Image {
        id: blur
        anchors.fill: parent
        visible: false
        source: tile.inView && !tile.sticker ? tile.blurPreview : ""
        fillMode: Image.PreserveAspectCrop
        asynchronous: true
        cache: false
    }

    Kirigami.ShadowedTexture {
        anchors.fill: parent
        radius: tile.radius
        source: blur
        color: "transparent"
        visible: !tile.ready && blur.status === Image.Ready
        opacity: 0.85
    }

    Item {
        id: shimmer
        anchors.fill: parent
        visible: !tile.ready
        clip: true

        Rectangle {
            id: sweep
            width: parent.width * 0.6
            height: parent.height * 2
            y: -parent.height / 2
            rotation: 18
            gradient: Gradient {
                orientation: Gradient.Horizontal
                GradientStop { position: 0; color: "transparent" }
                GradientStop { position: 0.5; color: Qt.alpha(Kirigami.Theme.textColor, 0.09) }
                GradientStop { position: 1; color: "transparent" }
            }
            NumberAnimation on x {
                from: -sweep.width
                to: shimmer.width + sweep.width
                duration: 1300 * Math.max(0.25, tile.animationScale)
                loops: Animation.Infinite
                running: shimmer.visible && tile.inView && tile.animationScale > 0
                easing.type: Easing.InOutQuad
            }
        }
    }

    Loader {
        id: imageLoader
        anchors.fill: parent
        active: tile.inView && tile.previewUrl.toString() !== ""
        visible: false
        sourceComponent: AnimatedImage {
            source: tile.previewUrl
            asynchronous: true
            cache: false
            fillMode: tile.sticker ? Image.PreserveAspectFit : Image.PreserveAspectCrop
            sourceSize.width: Math.round(tile.width * Screen.devicePixelRatio)
            playing: tile.playing
            smooth: true
        }
    }

    Kirigami.ShadowedTexture {
        id: rounded
        anchors.fill: parent
        radius: tile.radius
        source: tile.image
        color: "transparent"
        visible: tile.ready
        opacity: tile.ready ? 1 : 0
        Behavior on opacity {
            NumberAnimation { duration: Kirigami.Units.longDuration * tile.animationScale; easing.type: Easing.OutCubic }
        }
    }

    Rectangle {
        anchors.fill: parent
        radius: tile.radius
        color: Kirigami.Theme.highlightColor
        opacity: tap.pressed ? 0.18 : 0
        Behavior on opacity {
            NumberAnimation { duration: Kirigami.Units.shortDuration * tile.animationScale }
        }
    }

    Rectangle {
        id: favoriteBadge
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: Kirigami.Units.smallSpacing
        width: Kirigami.Units.iconSizes.small + Kirigami.Units.smallSpacing * 2
        height: width
        radius: width / 2
        color: Qt.alpha(Kirigami.Theme.backgroundColor, 0.75)
        visible: scale > 0.01
        scale: tile.favorite ? 1 : 0
        Behavior on scale {
            NumberAnimation { duration: Kirigami.Units.longDuration * tile.animationScale; easing.type: Easing.OutBack; easing.overshoot: 2.2 }
        }

        Kirigami.Icon {
            anchors.centerIn: parent
            width: Kirigami.Units.iconSizes.small
            height: width
            source: "emblem-favorite"
            color: Kirigami.Theme.negativeTextColor
            isMask: true
        }
    }

    Rectangle {
        anchors.fill: parent
        radius: tile.radius
        color: Qt.alpha(Kirigami.Theme.backgroundColor, 0.55)
        opacity: tile.inserting ? 1 : (tile.busy ? 0.35 : 0)
        visible: opacity > 0
        Behavior on opacity {
            NumberAnimation { duration: Kirigami.Units.longDuration * tile.animationScale; easing.type: Easing.OutCubic }
        }

        QQC2.BusyIndicator {
            anchors.centerIn: parent
            running: tile.inserting
            visible: tile.inserting
            width: Math.min(parent.width, parent.height, Kirigami.Units.iconSizes.large)
            height: width
        }
    }

    TapHandler {
        id: tap
        enabled: !tile.busy
        longPressThreshold: 0.45
        onTapped: tile.activated()
        onLongPressed: tile.favoriteToggled()
    }
}
