import QtQuick
import org.kde.kirigami as Kirigami
import org.devl0rd.kboard.platform
import org.devl0rd.kboard.config

PanelWindow {
    id: window

    readonly property real keyboardHeight: Math.round(screenHeight * (portrait ? Settings.heightPortrait : Settings.heightLandscape))

    width: screenWidth
    height: keyboardHeight
    visible: false
    interactiveRects: [Qt.rect(0, 0, width, height)]
    blurRect: Qt.rect(0, 0, width, height)

    Component.onCompleted: {
        if (!initPanel()) {
            console.error("KBoard must be launched by KWin as the virtual keyboard")
            Qt.exit(1)
        }
    }

    Connections {
        target: InputContext
        function onActivated() { window.visible = true }
        function onDeactivated() { window.visible = false }
    }

    Rectangle {
        anchors.fill: parent
        color: Qt.alpha(Kirigami.Theme.backgroundColor, 0.9)

        Grid {
            anchors.centerIn: parent
            columns: 10
            spacing: 6
            Repeater {
                model: "qwertyuiopasdfghjklzxcvbnm".split("")
                delegate: Rectangle {
                    required property string modelData
                    width: window.width / 11
                    height: window.height / 4
                    radius: 8
                    color: area.pressed ? Kirigami.Theme.highlightColor : Qt.alpha(Kirigami.Theme.textColor, 0.12)
                    Text {
                        anchors.centerIn: parent
                        text: parent.modelData
                        color: Kirigami.Theme.textColor
                        font.pixelSize: parent.height * 0.4
                    }
                    MouseArea {
                        id: area
                        anchors.fill: parent
                        onClicked: InputContext.commit(parent.modelData)
                    }
                }
            }
        }
    }
}
