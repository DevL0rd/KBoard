pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.devl0rd.kboard.emoji

Item {
    id: popup

    property var options: []
    property int hoverIndex: -1
    property bool open: false
    property real optionSize: 44
    property real kaomojiWidth: 160
    property real arrowX: width / 2
    property real animScale: 1
    property var source: null
    readonly property real padding: Kirigami.Units.smallSpacing * 1.5
    readonly property real arrowSize: Math.round(optionSize * 0.22)

    signal picked(var option, var cell)

    function showFor(cell, container, margin) {
        const list = []
        const variants = cell.tones ? EmojiStore.skinToneVariants(cell.base) : [cell.text]
        for (const variant of variants) {
            list.push({ kind: cell.kaomoji ? "kaomoji" : "emoji", text: variant })
        }
        list.push({ kind: "favorite", favorite: EmojiStore.isFavorite(cell.base) })
        options = list
        hoverIndex = Math.max(0, list.findIndex(option => option.text === cell.text))
        source = cell
        const anchor = cell.mapToItem(container, cell.width / 2, 0)
        x = Math.round(Math.max(margin, Math.min(container.width - width - margin, anchor.x - width / 2)))
        y = Math.round(Math.max(0, anchor.y - height - arrowSize))
        arrowX = anchor.x - x
        open = true
    }

    function track(cell, px, py) {
        const point = cell.mapToItem(popup, px, py)
        hoverIndex = indexAt(point.x, point.y)
    }

    function release(cell) {
        if (cell.moved || hoverIndex < 0) {
            choose(hoverIndex)
        }
    }

    function dismiss() {
        open = false
        source = null
    }

    function choose(index) {
        const option = options[index]
        const cell = source
        dismiss()
        if (option && cell) {
            picked(option, cell)
        }
    }

    function optionWidth(option) {
        return option.kind === "kaomoji" ? kaomojiWidth : optionSize
    }

    function indexAt(px, py) {
        if (py < -optionSize * 1.4 || py > height + optionSize * 1.6) {
            return -1
        }
        let x = padding
        for (let i = 0; i < options.length; ++i) {
            const w = optionWidth(options[i])
            if (px < x + w || i === options.length - 1) {
                return i
            }
            x += w
        }
        return -1
    }

    width: row.implicitWidth + padding * 2
    height: optionSize + padding * 2
    visible: opacity > 0.01
    opacity: open ? 1 : 0
    scale: open ? 1 : 0.82
    transformOrigin: Item.Bottom

    Behavior on opacity { NumberAnimation { duration: Math.round((popup.open ? 140 : 110) * popup.animScale); easing.type: Easing.OutCubic } }
    Behavior on scale { NumberAnimation { duration: Math.round((popup.open ? 240 : 110) * popup.animScale); easing.type: popup.open ? Easing.OutBack : Easing.InCubic } }

    Kirigami.ShadowedRectangle {
        id: background
        anchors.fill: parent
        radius: Math.round(popup.optionSize * 0.36)
        color: Kirigami.Theme.backgroundColor
        border.width: 1
        border.color: Qt.alpha(Kirigami.Theme.textColor, 0.14)
        shadow.size: Kirigami.Units.gridUnit
        shadow.yOffset: 2
        shadow.color: Qt.rgba(0, 0, 0, 0.32)
    }

    Rectangle {
        x: Math.max(background.radius, Math.min(popup.width - background.radius, popup.arrowX)) - width / 2
        y: popup.height - height / 2 - 1
        width: popup.arrowSize * 1.4
        height: width
        rotation: 45
        color: Kirigami.Theme.backgroundColor
        border.width: 1
        border.color: Qt.alpha(Kirigami.Theme.textColor, 0.14)

        Rectangle {
            x: -1
            y: -1
            width: parent.width * 0.6
            height: parent.height * 0.6
            color: Kirigami.Theme.backgroundColor
        }
    }

    Row {
        id: row
        x: popup.padding
        y: popup.padding

        Repeater {
            model: popup.options

            MouseArea {
                id: option

                required property var modelData
                required property int index
                readonly property bool hovered: index === popup.hoverIndex

                width: popup.optionWidth(modelData)
                height: popup.optionSize
                onClicked: popup.choose(index)

                Rectangle {
                    anchors.centerIn: parent
                    width: option.modelData.kind === "kaomoji" ? parent.width - 4 : parent.height - 4
                    height: parent.height - 4
                    radius: height / 2
                    color: Kirigami.Theme.highlightColor
                    opacity: option.hovered ? 0.3 : 0
                    scale: option.hovered ? 1 : 0.7
                    Behavior on opacity { NumberAnimation { duration: Math.round(110 * popup.animScale) } }
                    Behavior on scale { NumberAnimation { duration: Math.round(160 * popup.animScale); easing.type: Easing.OutCubic } }
                }

                Rectangle {
                    visible: option.modelData.kind === "favorite"
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    width: 1
                    height: parent.height * 0.6
                    color: Qt.alpha(Kirigami.Theme.textColor, 0.15)
                }

                Text {
                    visible: option.modelData.kind === "emoji"
                    anchors.centerIn: parent
                    text: option.modelData.kind === "emoji" ? option.modelData.text : ""
                    font.family: "Noto Color Emoji"
                    font.pixelSize: Math.round(popup.optionSize * 0.6)
                    scale: option.hovered ? 1.12 : 1
                    Behavior on scale { NumberAnimation { duration: Math.round(160 * popup.animScale); easing.type: Easing.OutBack } }
                }

                QQC2.Label {
                    visible: option.modelData.kind === "kaomoji"
                    anchors.fill: parent
                    anchors.margins: Kirigami.Units.smallSpacing
                    text: option.modelData.kind === "kaomoji" ? option.modelData.text : ""
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    fontSizeMode: Text.HorizontalFit
                    minimumPixelSize: 8
                    font.pixelSize: Math.round(popup.optionSize * 0.4)
                    color: Kirigami.Theme.textColor
                }

                Kirigami.Icon {
                    visible: option.modelData.kind === "favorite"
                    anchors.centerIn: parent
                    width: Math.round(popup.optionSize * 0.46)
                    height: width
                    source: option.modelData.favorite ? "favorite-favorited" : "favorite"
                    color: option.modelData.favorite ? Kirigami.Theme.highlightColor : Kirigami.Theme.textColor
                    scale: option.hovered ? 1.15 : 1
                    Behavior on scale { NumberAnimation { duration: Math.round(160 * popup.animScale); easing.type: Easing.OutBack } }
                }
            }
        }
    }
}
