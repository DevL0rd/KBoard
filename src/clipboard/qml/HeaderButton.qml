import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

Item {
    id: button

    property string iconName
    property string tooltip
    property bool highlighted: false

    signal clicked()

    Layout.preferredWidth: Kirigami.Units.gridUnit * 2
    Layout.preferredHeight: Kirigami.Units.gridUnit * 2
    opacity: enabled ? 1 : 0.4

    Rectangle {
        anchors.fill: parent
        radius: width / 2
        color: button.highlighted ? Qt.alpha(Kirigami.Theme.neutralTextColor, 0.22)
            : tap.pressed ? Qt.alpha(Kirigami.Theme.textColor, 0.16)
            : hover.hovered ? Qt.alpha(Kirigami.Theme.textColor, 0.08) : "transparent"
        scale: tap.pressed ? 0.9 : 1
        Behavior on scale { NumberAnimation { duration: ClipboardStyle.duration(140); easing.type: Easing.OutCubic } }
        Behavior on color { ColorAnimation { duration: ClipboardStyle.duration(140) } }
    }

    Kirigami.Icon {
        anchors.centerIn: parent
        width: Kirigami.Units.iconSizes.smallMedium
        height: width
        source: button.iconName
        color: button.highlighted ? Kirigami.Theme.neutralTextColor : Kirigami.Theme.textColor
        isMask: true
    }

    HoverHandler { id: hover }

    TapHandler {
        id: tap
        onTapped: button.clicked()
    }

    QQC2.ToolTip.visible: hover.hovered && tooltip.length > 0
    QQC2.ToolTip.text: tooltip
    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
}
