import QtQuick
import QtQuick.Window
import org.kde.kirigami as Kirigami
import org.devl0rd.kboard.config
import org.devl0rd.kboard.clipboard

Window {
    id: window
    width: previewWidth
    height: previewHeight
    visible: true
    color: Kirigami.Theme.backgroundColor

    Rectangle {
        anchors.fill: parent
        color: Kirigami.ColorUtils.linearInterpolation(Kirigami.Theme.backgroundColor, Kirigami.Theme.textColor, 0.03)
    }

    ClipboardPanel {
        id: panel
        anchors.fill: parent
        onCloseRequested: console.log("closeRequested")
    }

    Timer {
        interval: 700
        running: true
        onTriggered: {
            switch (previewScenario) {
            case "pinned": panel.filter = "pinned"; break
            case "links": panel.filter = "links"; break
            case "armed": panel.requestClear(); break
            case "focus": panel.moveFocus(0, 0); panel.moveFocus(1, 0); panel.moveFocus(0, 1); break
            }
        }
    }
}
