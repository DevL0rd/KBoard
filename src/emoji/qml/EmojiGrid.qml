pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import QtQml.Models
import org.kde.kirigami as Kirigami
import org.devl0rd.kboard.emoji

ListView {
    id: grid

    required property EmojiGridModel gridModel
    property real cellWidth: 48
    property real cellHeight: 48
    property real glyphSize: 30
    property int kaomojiColumns: 3
    property real animScale: 1
    property bool searchActive: false
    property string activeSection: EmojiStore.recentList.length > 0 ? "recents" : "smileys"
    property bool jumping: false

    signal cellActivated(var cell)
    signal cellHeld(var cell)
    signal cellDragged(var cell, real x, real y)
    signal cellDropped(var cell)

    function jumpTo(id) {
        const row = gridModel.rowForSection(id)
        if (row < 0) {
            return
        }
        activeSection = id
        scrollAnimation.stop()
        fadeJump.stop()
        jumping = true
        const from = contentY
        positionViewAtIndex(row, ListView.Beginning)
        const distance = Math.abs(contentY - from)
        if (animScale === 0 || distance < 1) {
            jumping = false
            return
        }
        const animation = distance > height * 2.5 ? fadeJump : scrollAnimation
        scrollAnimation.to = contentY
        contentY = from
        animation.row = row
        animation.start()
    }

    function settle(row) {
        positionViewAtIndex(row, ListView.Beginning)
        jumping = false
    }

    function restoreSection() {
        if (searchActive || height <= 0 || count === 0) {
            return
        }
        const row = gridModel.rowForSection(activeSection)
        if (row >= 0) {
            jumping = true
            settle(row)
        }
    }

    function trackSection() {
        if (jumping || searchActive || count === 0) {
            return
        }
        if (atYEnd && contentHeight > height) {
            activeSection = gridModel.sectionAt(count - 1)
            return
        }
        const row = indexAt(Kirigami.Units.gridUnit, contentY + Kirigami.Units.gridUnit)
        if (row >= 0) {
            activeSection = gridModel.sectionAt(row)
        }
    }

    clip: true
    model: gridModel
    reuseItems: true
    cacheBuffer: Math.max(0, Math.round(cellHeight * 6))
    boundsBehavior: Flickable.StopAtBounds
    maximumFlickVelocity: 6000
    flickDeceleration: 2600

    onContentYChanged: trackSection()
    onHeightChanged: Qt.callLater(restoreSection)

    Connections {
        target: grid.gridModel
        function onSectionsChanged() {
            Qt.callLater(grid.restoreSection)
        }
    }

    QQC2.ScrollBar.vertical: QQC2.ScrollBar {
        policy: QQC2.ScrollBar.AsNeeded
    }

    NumberAnimation {
        id: scrollAnimation
        property int row: 0
        target: grid
        property: "contentY"
        duration: Math.round(320 * grid.animScale)
        easing.type: Easing.OutCubic
        onFinished: grid.settle(row)
    }

    SequentialAnimation {
        id: fadeJump
        property int row: 0
        NumberAnimation { target: grid; property: "opacity"; to: 0; duration: Math.round(90 * grid.animScale); easing.type: Easing.InCubic }
        ScriptAction { script: grid.settle(fadeJump.row) }
        NumberAnimation { target: grid; property: "opacity"; to: 1; duration: Math.round(160 * grid.animScale); easing.type: Easing.OutCubic }
    }

    delegate: DelegateChooser {
        role: "kind"

        DelegateChoice {
            roleValue: "header"

            QQC2.Label {
                required property string title
                width: grid.width
                height: Math.round(Math.max(Kirigami.Units.gridUnit * 1.25, grid.cellHeight * 0.44))
                leftPadding: Kirigami.Units.smallSpacing * 2
                bottomPadding: Kirigami.Units.smallSpacing
                verticalAlignment: Text.AlignBottom
                text: title
                color: Qt.alpha(Kirigami.Theme.textColor, 0.66)
                font.weight: Font.DemiBold
                font.pixelSize: Math.round(Math.max(11, Math.min(15, grid.cellHeight * 0.26)))
                font.letterSpacing: 0.3
            }
        }

        DelegateChoice {
            roleValue: "empty"

            Item {
                id: emptyRow
                required property string title
                width: grid.width
                height: grid.cellHeight

                Row {
                    anchors.centerIn: parent
                    spacing: Kirigami.Units.smallSpacing * 2
                    opacity: 0.55

                    Kirigami.Icon {
                        anchors.verticalCenter: parent.verticalCenter
                        width: Kirigami.Units.iconSizes.smallMedium
                        height: width
                        source: grid.searchActive ? "search" : "clock"
                        color: Kirigami.Theme.textColor
                    }

                    QQC2.Label {
                        anchors.verticalCenter: parent.verticalCenter
                        text: emptyRow.title
                        color: Kirigami.Theme.textColor
                    }
                }
            }
        }

        DelegateChoice {
            roleValue: "emoji"
            CellRow { view: grid }
        }

        DelegateChoice {
            roleValue: "kaomoji"
            CellRow { view: grid; kaomoji: true }
        }
    }
}
