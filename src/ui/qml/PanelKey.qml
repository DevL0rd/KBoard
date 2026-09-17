pragma ComponentBehavior: Bound

import QtQuick
import org.kde.kirigami as Kirigami

Item {
    id: key

    property string iconName
    property string glyph
    property string text
    property bool special: false
    property bool checked: false
    property bool autoRepeat: false
    readonly property bool pressed: area.pressed

    signal activated

    Rectangle {
        anchors.fill: face
        anchors.topMargin: Math.max(1, Math.round(key.height * 0.03))
        anchors.bottomMargin: -anchors.topMargin
        radius: face.radius
        color: Theme.keyShadow
        visible: Theme.raised
        scale: face.scale
    }

    Rectangle {
        id: face

        anchors.fill: parent
        anchors.margins: Theme.gap / 2
        radius: Math.min(Theme.radius, height / 2)
        color: key.checked ? Theme.accentSoft : area.pressed ? (key.special ? Theme.specialKeyPressed : Theme.keyPressed) : (key.special ? Theme.specialKey : Theme.key)
        border.width: key.checked ? 1.5 : Theme.bordered || Theme.glass ? 1 : 0
        border.color: key.checked ? Theme.accent : Theme.keyBorder
        scale: area.pressed ? 0.94 : 1

        Behavior on scale {
            NumberAnimation { duration: Theme.duration(area.pressed ? 70 : 280); easing.type: area.pressed ? Easing.OutCubic : Easing.OutBack; easing.overshoot: 3 }
        }
        Behavior on color {
            ColorAnimation { duration: Theme.duration(120) }
        }

        Column {
            anchors.centerIn: parent
            spacing: key.height * 0.04

            Kirigami.Icon {
                anchors.horizontalCenter: parent.horizontalCenter
                width: Math.round(Math.min(key.height * 0.34, key.width * 0.3))
                height: width
                visible: key.iconName !== ""
                source: key.iconName
                isMask: true
                color: key.checked ? Theme.accent : Theme.keyText
            }

            KeyIcon {
                anchors.horizontalCenter: parent.horizontalCenter
                width: Math.round(Math.min(key.height * 0.42, key.width * 0.36))
                height: width
                visible: key.glyph !== ""
                name: key.glyph
                color: key.checked ? Theme.accent : Theme.keyText
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                visible: key.text !== ""
                text: key.text
                color: key.checked ? Theme.accent : Theme.keyText
                font.family: Theme.font.family
                font.pixelSize: Math.max(9, Math.round(Math.min(key.height * 0.2, key.width * 0.16) * Theme.labelScale))
                font.weight: Font.Medium
            }
        }
    }

    MouseArea {
        id: area

        anchors.fill: parent
        onPressed: {
            key.activated()
            if (key.autoRepeat)
                repeat.start()
        }
        onReleased: repeat.stop()
        onCanceled: repeat.stop()
    }

    Timer {
        id: repeat

        interval: 380
        repeat: true
        onTriggered: {
            interval = Math.max(35, interval * 0.7)
            key.activated()
        }
        onRunningChanged: if (!running) interval = 380
    }
}
