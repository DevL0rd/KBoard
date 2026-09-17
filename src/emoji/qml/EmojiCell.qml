import QtQuick
import QtQuick.Shapes
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.devl0rd.kboard.config

MouseArea {
    id: cell

    property string text
    property string base
    property string name
    property bool tones: false
    property bool favorite: false
    property bool kaomoji: false
    property real glyphSize: 24
    property real animScale: 1
    property bool holding: false
    property real pressX: 0
    property real pressY: 0
    property bool moved: false

    signal activated(var cell)
    signal held(var cell)
    signal dragged(var cell, real x, real y)
    signal dropped(var cell)

    function pop() {
        popAnimation.restart()
    }

    pressAndHoldInterval: Settings.longPressDelay
    cursorShape: Qt.PointingHandCursor
    Accessible.role: Accessible.Button
    Accessible.name: cell.name !== "" ? cell.name : cell.text

    onPressed: mouse => {
        pressX = mouse.x
        pressY = mouse.y
        moved = false
    }
    onPressAndHold: {
        holding = true
        preventStealing = true
        held(cell)
    }
    onPositionChanged: mouse => {
        if (!holding) {
            return
        }
        if (Math.abs(mouse.x - pressX) > Kirigami.Units.gridUnit * 0.5 || Math.abs(mouse.y - pressY) > Kirigami.Units.gridUnit * 0.5) {
            moved = true
        }
        dragged(cell, mouse.x, mouse.y)
    }
    onReleased: {
        if (holding) {
            holding = false
            preventStealing = false
            dropped(cell)
        }
    }
    onCanceled: {
        holding = false
        preventStealing = false
    }
    onClicked: {
        pop()
        activated(cell)
    }

    Rectangle {
        anchors.centerIn: parent
        width: cell.kaomoji ? parent.width - Kirigami.Units.smallSpacing * 2 : Math.min(parent.width, parent.height) - Kirigami.Units.smallSpacing
        height: cell.kaomoji ? parent.height - Kirigami.Units.smallSpacing * 2 : width
        radius: cell.kaomoji ? height / 2 : width / 2
        color: cell.kaomoji ? Qt.alpha(Kirigami.Theme.textColor, cell.pressed || cell.holding ? 0.16 : 0.06) : Qt.alpha(Kirigami.Theme.textColor, 0.14)
        opacity: cell.kaomoji || cell.pressed || cell.holding ? 1 : 0
        scale: cell.pressed || cell.holding ? 1 : 0.8
        Behavior on opacity { NumberAnimation { duration: Math.round(120 * cell.animScale) } }
        Behavior on scale { NumberAnimation { duration: Math.round(160 * cell.animScale); easing.type: Easing.OutCubic } }
        Behavior on color { ColorAnimation { duration: Math.round(120 * cell.animScale) } }
    }

    Text {
        id: glyph
        visible: !cell.kaomoji
        anchors.centerIn: parent
        text: cell.kaomoji ? "" : cell.text
        font.family: "Noto Color Emoji"
        font.pixelSize: Math.round(cell.glyphSize)
        renderType: Text.QtRendering
    }

    QQC2.Label {
        id: kaomojiLabel
        visible: cell.kaomoji
        anchors.fill: parent
        anchors.leftMargin: Kirigami.Units.largeSpacing
        anchors.rightMargin: Kirigami.Units.largeSpacing
        text: cell.kaomoji ? cell.text : ""
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        fontSizeMode: Text.HorizontalFit
        minimumPixelSize: Math.round(cell.glyphSize * 0.36)
        font.pixelSize: Math.round(cell.glyphSize * 0.62)
        color: Kirigami.Theme.textColor
        elide: Text.ElideRight
        maximumLineCount: 1
    }

    Shape {
        id: toneMark
        visible: cell.tones
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.rightMargin: Math.round(parent.width * 0.12)
        anchors.bottomMargin: Math.round(parent.height * 0.12)
        width: Math.max(4, Math.round(Math.min(parent.width, parent.height) * 0.1))
        height: width
        preferredRendererType: Shape.CurveRenderer

        ShapePath {
            strokeWidth: -1
            fillColor: Qt.alpha(Kirigami.Theme.textColor, 0.35)
            startX: toneMark.width
            startY: 0
            PathLine { x: toneMark.width; y: toneMark.height }
            PathLine { x: 0; y: toneMark.height }
            PathLine { x: toneMark.width; y: 0 }
        }
    }

    Kirigami.Icon {
        visible: cell.favorite && cell.kaomoji
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: Kirigami.Units.smallSpacing * 1.5
        width: Math.round(cell.glyphSize * 0.32)
        height: width
        source: "favorite-favorited"
    }

    SequentialAnimation {
        id: popAnimation
        NumberAnimation { targets: [glyph, kaomojiLabel]; property: "scale"; to: 1.32; duration: Math.round(80 * cell.animScale); easing.type: Easing.OutCubic }
        NumberAnimation { targets: [glyph, kaomojiLabel]; property: "scale"; to: 1; duration: Math.round(220 * cell.animScale); easing.type: Easing.OutBack }
    }
}
