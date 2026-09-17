pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

QQC2.ToolButton {
    id: badge

    signal resetRequested

    icon.name: "edit-undo"
    icon.width: Kirigami.Units.iconSizes.small
    icon.height: Kirigami.Units.iconSizes.small
    display: QQC2.AbstractButton.IconOnly
    text: "Reset to default"
    padding: 2
    onClicked: resetRequested()
    Accessible.description: "Changed from the default. Activate to reset."

    QQC2.ToolTip.text: "Changed from the default. Click to reset."
    QQC2.ToolTip.visible: hovered || visualFocus
    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay

    Rectangle {
        width: 6
        height: 6
        radius: 3
        color: Kirigami.Theme.highlightColor
        anchors.right: parent.right
        anchors.top: parent.top
    }
}
