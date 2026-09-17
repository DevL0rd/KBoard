pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Effects
import org.kde.kirigami as Kirigami

Item {
    id: bar

    property var groups: []
    property string currentId
    property real animScale: 1
    readonly property int currentIndex: {
        for (let i = 0; i < groups.length; ++i) {
            if (groups[i].id === currentId) {
                return i
            }
        }
        return -1
    }

    signal activated(string id)

    onCurrentIndexChanged: {
        if (currentIndex < 0 || !list.currentItem) {
            return
        }
        const item = list.currentItem
        const target = Math.max(0, Math.min(list.contentWidth - list.width, item.x + item.width / 2 - list.width / 2))
        if (Math.abs(target - list.contentX) < 1) {
            return
        }
        scroller.to = target
        scroller.restart()
    }

    NumberAnimation {
        id: scroller
        target: list
        property: "contentX"
        duration: Math.round(280 * bar.animScale)
        easing.type: Easing.OutCubic
    }

    ListView {
        id: list

        readonly property real tabWidth: Math.max(Math.round(height * 1.2), width / Math.max(1, count))

        anchors.fill: parent
        orientation: ListView.Horizontal
        model: bar.groups
        currentIndex: bar.currentIndex
        interactive: contentWidth > width + 1
        boundsBehavior: Flickable.StopAtBounds
        highlightFollowsCurrentItem: false
        clip: true

        highlight: Item {
            x: list.currentItem ? list.currentItem.x : 0
            width: list.tabWidth
            height: list.height
            visible: list.currentItem !== null

            Behavior on x {
                NumberAnimation { duration: Math.round(300 * bar.animScale); easing.type: Easing.OutCubic }
            }

            Rectangle {
                anchors.centerIn: parent
                anchors.verticalCenterOffset: -1
                width: Math.min(parent.width - Kirigami.Units.smallSpacing, parent.height * 1.5)
                height: parent.height - Kirigami.Units.smallSpacing * 2 - 2
                radius: height / 2
                color: Qt.alpha(Kirigami.Theme.highlightColor, 0.18)
            }

            Rectangle {
                anchors.bottom: parent.bottom
                anchors.horizontalCenter: parent.horizontalCenter
                width: Math.round(Math.min(parent.width, parent.height * 1.5) * 0.42)
                height: 3
                radius: 1.5
                color: Kirigami.Theme.highlightColor
            }
        }

        delegate: MouseArea {
            id: tab

            required property var modelData
            required property int index
            readonly property bool current: index === bar.currentIndex
            readonly property bool emojiIcon: modelData.id !== "kaomoji"

            width: list.tabWidth
            height: list.height
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: bar.activated(modelData.id)

            Rectangle {
                anchors.centerIn: parent
                anchors.verticalCenterOffset: -1
                width: Math.min(parent.width - Kirigami.Units.smallSpacing, parent.height * 1.5)
                height: parent.height - Kirigami.Units.smallSpacing * 2 - 2
                radius: height / 2
                color: Qt.alpha(Kirigami.Theme.textColor, tab.pressed ? 0.12 : 0.06)
                opacity: (tab.containsMouse || tab.pressed) && !tab.current ? 1 : 0
                Behavior on opacity { NumberAnimation { duration: Math.round(120 * bar.animScale) } }
            }

            Text {
                id: glyph
                anchors.centerIn: parent
                anchors.verticalCenterOffset: -1
                text: tab.modelData.icon
                font.family: tab.emojiIcon ? "Noto Color Emoji" : Kirigami.Theme.defaultFont.family
                font.pixelSize: Math.round(tab.height * (tab.emojiIcon ? 0.62 : 0.52))
                font.weight: Font.Bold
                color: tab.current ? Kirigami.Theme.highlightColor : Kirigami.Theme.textColor
                opacity: tab.current ? 1 : 0.85
                scale: tab.pressed ? 0.88 : (tab.current ? 1.08 : 1)
                layer.enabled: tab.emojiIcon
                layer.effect: MultiEffect {
                    saturation: tab.current ? 0 : -0.9
                    Behavior on saturation { NumberAnimation { duration: Math.round(220 * bar.animScale) } }
                }
                Behavior on opacity { NumberAnimation { duration: Math.round(220 * bar.animScale) } }
                Behavior on scale { NumberAnimation { duration: Math.round(220 * bar.animScale); easing.type: Easing.OutBack } }
            }
        }
    }
}
