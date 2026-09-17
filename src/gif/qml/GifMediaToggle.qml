pragma ComponentBehavior: Bound

import QtQuick
import org.kde.kirigami as Kirigami
import org.devl0rd.kboard.gif

Rectangle {
    id: toggle

    property real animationScale: 1

    readonly property var options: [
        { type: GifStore.Gifs, label: qsTr("GIFs") },
        { type: GifStore.Stickers, label: qsTr("Stickers") }
    ]

    implicitWidth: row.implicitWidth
    radius: height / 2
    color: Qt.alpha(Kirigami.Theme.textColor, 0.07)

    Rectangle {
        readonly property Item target: repeater.count > 0 ? repeater.itemAt(GifStore.mediaType === GifStore.Stickers ? 1 : 0) : null
        x: target ? target.x + 3 : 0
        width: target ? target.width - 6 : 0
        y: 3
        height: parent.height - 6
        radius: height / 2
        color: Kirigami.Theme.highlightColor

        Behavior on x {
            NumberAnimation { duration: Kirigami.Units.longDuration * toggle.animationScale; easing.type: Easing.OutCubic }
        }
        Behavior on width {
            NumberAnimation { duration: Kirigami.Units.longDuration * toggle.animationScale; easing.type: Easing.OutCubic }
        }
    }

    Row {
        id: row
        height: parent.height

        Repeater {
            id: repeater
            model: toggle.options

            delegate: Item {
                id: cell
                required property var modelData
                readonly property bool selected: GifStore.mediaType === modelData.type
                width: cellLabel.implicitWidth + Kirigami.Units.gridUnit * 1.2
                height: row.height

                Text {
                    id: cellLabel
                    anchors.centerIn: parent
                    text: cell.modelData.label
                    font.weight: Font.DemiBold
                    color: cell.selected ? Kirigami.Theme.highlightedTextColor : Kirigami.Theme.textColor
                }

                TapHandler {
                    onTapped: GifStore.mediaType = cell.modelData.type
                }
            }
        }
    }
}
