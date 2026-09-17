import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents

MouseArea {
    id: tile

    property string label
    property string iconName
    property string hint
    property bool current: false

    signal activated()

    Layout.fillWidth: true
    implicitHeight: column.implicitHeight + Kirigami.Units.largeSpacing * 2
    implicitWidth: Kirigami.Units.gridUnit * 5
    hoverEnabled: true
    cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
    opacity: enabled ? 1 : 0.45
    activeFocusOnTab: true
    Keys.onReturnPressed: tile.activated()
    Keys.onSpacePressed: tile.activated()
    onClicked: {
        ripple.play()
        tile.activated()
    }

    QQC2.ToolTip.visible: containsMouse && hint !== ""
    QQC2.ToolTip.text: hint
    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay

    Rectangle {
        id: background
        anchors.fill: parent
        radius: Kirigami.Units.cornerRadius * 2
        color: tile.current ? Qt.alpha(Kirigami.Theme.highlightColor, tile.pressed ? 0.34 : 0.22)
             : Qt.alpha(tile.containsMouse || tile.activeFocus ? Kirigami.Theme.highlightColor : Kirigami.Theme.textColor,
                        tile.pressed ? 0.24 : tile.containsMouse || tile.activeFocus ? 0.12 : 0.045)
        border.width: 1
        border.color: tile.current || tile.activeFocus ? Qt.alpha(Kirigami.Theme.highlightColor, 0.8) : Qt.alpha(Kirigami.Theme.textColor, 0.07)
        scale: tile.pressed ? 0.96 : 1
        Behavior on color { ColorAnimation { duration: Kirigami.Units.shortDuration } }
        Behavior on border.color { ColorAnimation { duration: Kirigami.Units.shortDuration } }
        Behavior on scale { NumberAnimation { duration: Kirigami.Units.shortDuration; easing.type: Easing.OutCubic } }
    }

    Item {
        anchors.fill: parent
        clip: true

        Rectangle {
            id: ripple
            property real originX: tile.mouseX
            property real originY: tile.mouseY
            x: originX - width / 2
            y: originY - height / 2
            width: Math.hypot(tile.width, tile.height) * 2
            height: width
            radius: width / 2
            color: Qt.alpha(Kirigami.Theme.highlightColor, 0.3)
            opacity: 0
            scale: 0

            function play() {
                originX = tile.mouseX
                originY = tile.mouseY
                wave.restart()
            }

            ParallelAnimation {
                id: wave
                NumberAnimation { target: ripple; property: "scale"; from: 0; to: 1; duration: Kirigami.Units.veryLongDuration; easing.type: Easing.OutCubic }
                NumberAnimation { target: ripple; property: "opacity"; from: 1; to: 0; duration: Kirigami.Units.veryLongDuration; easing.type: Easing.InQuad }
            }
        }
    }

    ColumnLayout {
        id: column
        anchors.centerIn: parent
        width: parent.width - Kirigami.Units.smallSpacing * 2
        spacing: Kirigami.Units.smallSpacing

        Kirigami.Icon {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: Kirigami.Units.iconSizes.medium
            Layout.preferredHeight: Kirigami.Units.iconSizes.medium
            source: tile.iconName
            active: tile.containsMouse
            scale: tile.containsMouse ? 1.08 : 1
            Behavior on scale { NumberAnimation { duration: Kirigami.Units.shortDuration; easing.type: Easing.OutCubic } }
        }
        PlasmaComponents.Label {
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            text: tile.label
            elide: Text.ElideRight
            font.weight: tile.current ? Font.DemiBold : Font.Normal
        }
    }
}
