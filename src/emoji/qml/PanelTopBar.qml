import QtQuick
import org.kde.kirigami as Kirigami
import org.devl0rd.kboard.config

Item {
    id: bar

    property string searchText
    property bool searchActive: false
    property bool showBackspace: true
    property real animScale: 1

    signal backRequested()
    signal searchRequested()
    signal clearRequested()
    signal backspaceRequested()

    PanelButton {
        id: leftButton
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: bar.searchActive ? height : Math.round(height * 1.7)
        text: bar.searchActive ? "" : "ABC"
        iconName: bar.searchActive ? "go-previous" : ""
        animScale: bar.animScale
        onTriggered: bar.backRequested()
        Behavior on width { NumberAnimation { duration: Math.round(200 * bar.animScale); easing.type: Easing.OutCubic } }
    }

    SearchPill {
        anchors.left: leftButton.right
        anchors.right: backspaceButton.visible ? backspaceButton.left : parent.right
        anchors.leftMargin: Kirigami.Units.smallSpacing * 2
        anchors.rightMargin: backspaceButton.visible ? Kirigami.Units.smallSpacing * 2 : 0
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        text: bar.searchText
        active: bar.searchActive
        animScale: bar.animScale
        onClicked: {
            if (!bar.searchActive) {
                bar.searchRequested()
            }
        }
        onCleared: bar.clearRequested()
    }

    PanelButton {
        id: backspaceButton
        visible: bar.showBackspace
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: Math.round(height * 1.5)
        iconName: "edit-clear-locationbar-rtl"
        autoRepeat: true
        repeatDelay: Settings.keyRepeatDelay
        repeatInterval: Settings.keyRepeatInterval
        animScale: bar.animScale
        onTriggered: bar.backspaceRequested()
    }
}
