pragma ComponentBehavior: Bound

import QtQuick
import org.kde.kirigami as Kirigami

Item {
    id: button

    property string iconName
    property string glyph
    property string text
    property bool active: false
    property bool highlighted: false

    signal clicked

    implicitWidth: height * 1.2
    implicitHeight: 40

    Rectangle {
        anchors.centerIn: parent
        width: Math.min(parent.width, parent.height * 1.1)
        height: parent.height * 0.8
        radius: height / 2
        color: button.active ? Theme.accentSoft : area.pressed ? Qt.alpha(Theme.text, 0.12) : "transparent"
        scale: area.pressed ? 0.9 : 1

        Behavior on color {
            ColorAnimation { duration: Theme.duration(120) }
        }
        Behavior on scale {
            NumberAnimation { duration: Theme.duration(160); easing.type: Easing.OutBack }
        }
    }

    KeyIcon {
        anchors.centerIn: parent
        width: Math.round(parent.height * 0.52)
        height: width
        visible: button.glyph !== ""
        name: button.glyph
        color: button.active ? Theme.accent : Qt.alpha(Theme.text, 0.82)
    }

    Kirigami.Icon {
        anchors.centerIn: parent
        width: Math.round(parent.height * 0.5)
        height: width
        visible: button.iconName !== ""
        source: button.iconName
        isMask: !Theme.dark
        color: button.active ? Theme.accent : Qt.alpha(Theme.text, 0.82)
    }

    Text {
        anchors.centerIn: parent
        visible: button.iconName === "" && button.glyph === ""
        text: button.text
        color: button.active || button.highlighted ? Theme.accent : Qt.alpha(Theme.text, 0.82)
        font.pixelSize: Math.round(parent.height * 0.3)
        font.weight: Font.Bold
        font.letterSpacing: 0.5
    }

    MouseArea {
        id: area

        anchors.fill: parent
        onClicked: button.clicked()
    }
}
