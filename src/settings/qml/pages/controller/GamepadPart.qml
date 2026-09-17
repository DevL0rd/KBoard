pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

Rectangle {
    id: part

    property string button
    property var lit: []
    property string glyph
    property color accent: Kirigami.Theme.highlightColor
    readonly property bool on: lit.indexOf(button) >= 0

    color: on ? accent : Qt.alpha(Kirigami.Theme.textColor, 0.14)
    border.width: 1
    border.color: on ? Qt.lighter(accent, 1.3) : Qt.alpha(Kirigami.Theme.textColor, 0.3)
    scale: on ? 0.92 : 1

    Behavior on color {
        ColorAnimation {
            duration: Motion.quick
        }
    }

    Behavior on scale {
        NumberAnimation {
            duration: Motion.quick
            easing.type: Easing.OutCubic
        }
    }

    QQC2.Label {
        anchors.centerIn: parent
        text: part.glyph
        visible: text.length > 0
        font.pixelSize: Math.max(6, Math.min(parent.width, parent.height) * 0.55)
        font.weight: Font.DemiBold
        color: part.on ? Kirigami.Theme.highlightedTextColor : Kirigami.Theme.textColor
    }
}
