import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

MouseArea {
    id: button

    property string iconName
    property string text
    property bool highlighted: false
    property bool autoRepeat: false
    property int repeatDelay: 400
    property int repeatInterval: 50
    property real animScale: 1

    signal triggered()

    implicitWidth: Math.max(height, label.implicitWidth + Kirigami.Units.largeSpacing * 2.5)
    hoverEnabled: true
    cursorShape: Qt.PointingHandCursor

    onPressed: {
        if (autoRepeat) {
            triggered()
            repeatTimer.interval = repeatDelay
            repeatTimer.start()
        }
    }
    onReleased: repeatTimer.stop()
    onCanceled: repeatTimer.stop()
    onClicked: {
        if (!autoRepeat) {
            triggered()
        }
    }

    Timer {
        id: repeatTimer
        repeat: true
        onTriggered: {
            interval = button.repeatInterval
            button.triggered()
        }
    }

    Rectangle {
        anchors.fill: parent
        radius: height / 2
        color: button.highlighted
            ? Qt.alpha(Kirigami.Theme.highlightColor, button.pressed ? 0.42 : 0.26)
            : Qt.alpha(Kirigami.Theme.textColor, button.pressed ? 0.2 : (button.containsMouse ? 0.11 : 0.07))
        scale: button.pressed ? 0.93 : 1
        Behavior on color { ColorAnimation { duration: Math.round(110 * button.animScale) } }
        Behavior on scale { NumberAnimation { duration: Math.round(160 * button.animScale); easing.type: Easing.OutCubic } }
    }

    Kirigami.Icon {
        visible: button.iconName !== ""
        anchors.centerIn: parent
        width: Math.round(Math.min(parent.height, parent.width) * 0.5)
        height: width
        source: button.iconName
        color: Kirigami.Theme.textColor
    }

    QQC2.Label {
        id: label
        visible: button.text !== ""
        anchors.centerIn: parent
        text: button.text
        color: Kirigami.Theme.textColor
        font.weight: Font.DemiBold
        font.pixelSize: Math.round(button.height * 0.34)
    }
}
