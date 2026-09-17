pragma ComponentBehavior: Bound

import QtQuick
import org.kde.kirigami as Kirigami

Item {
    id: chip

    property var entry: ({})
    property bool shown: false
    readonly property bool image: entry.kind === "image"
    readonly property string text: entry.otpCode ? entry.otpCode : image ? qsTr("Image") : (entry.text ?? "")

    signal clicked
    signal dismissed

    opacity: shown ? 1 : 0
    visible: opacity > 0
    implicitHeight: 40

    Behavior on opacity {
        NumberAnimation { duration: Theme.duration(200); easing.type: Easing.OutCubic }
    }

    Rectangle {
        id: pill

        anchors.verticalCenter: parent.verticalCenter
        width: Math.min(parent.width, row.implicitWidth + chip.height * 0.6)
        height: chip.height * 0.72
        radius: height / 2
        color: area.pressed ? Qt.alpha(Theme.accent, 0.35) : Theme.accentSoft
        border.width: 1
        border.color: Qt.alpha(Theme.accent, 0.5)
        scale: chip.shown ? 1 : 0.85

        Behavior on scale {
            NumberAnimation { duration: Theme.duration(260); easing.type: Easing.OutBack }
        }

        Row {
            id: row

            anchors.centerIn: parent
            spacing: chip.height * 0.15

            Kirigami.Icon {
                anchors.verticalCenter: parent.verticalCenter
                width: Math.round(chip.height * 0.4)
                height: width
                source: chip.image ? chip.entry.thumbnailSource : chip.entry.otpCode ? "dialog-password" : "edit-paste"
                isMask: !chip.image
                color: Theme.accent
            }

            Text {
                anchors.verticalCenter: parent.verticalCenter
                width: Math.min(implicitWidth, pill.parent.width - chip.height * 1.9)
                text: chip.text.replace(/\s+/g, " ")
                elide: Text.ElideRight
                color: Theme.text
                font.pixelSize: Math.round(chip.height * 0.32)
                font.family: chip.entry.otpCode ? "monospace" : Theme.font.family
            }

            Kirigami.Icon {
                anchors.verticalCenter: parent.verticalCenter
                width: Math.round(chip.height * 0.3)
                height: width
                source: "window-close"
                isMask: true
                color: Qt.alpha(Theme.text, 0.6)

                TapHandler {
                    onTapped: chip.dismissed()
                }
            }
        }
    }

    MouseArea {
        id: area

        anchors.fill: pill
        anchors.rightMargin: chip.height * 0.5
        enabled: chip.shown
        onClicked: chip.clicked()
    }
}
