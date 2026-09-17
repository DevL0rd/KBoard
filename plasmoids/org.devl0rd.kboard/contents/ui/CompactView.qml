import QtQuick
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.plasma.plasmoid
import org.kde.plasma.core as PlasmaCore

MouseArea {
    id: compact

    readonly property bool vertical: Plasmoid.formFactor === PlasmaCore.Types.Vertical
    readonly property real thickness: vertical ? width : height
    readonly property real iconSize: Math.round(Math.min(width, height) * 0.62)
    property bool wasExpanded: false

    Layout.minimumWidth: vertical ? 0 : height
    Layout.maximumWidth: vertical ? -1 : height
    Layout.preferredWidth: Layout.minimumWidth
    Layout.minimumHeight: vertical ? width : 0
    Layout.maximumHeight: vertical ? width : -1
    Layout.preferredHeight: Layout.minimumHeight
    implicitWidth: Kirigami.Units.iconSizes.medium
    implicitHeight: Kirigami.Units.iconSizes.medium

    acceptedButtons: Qt.LeftButton | Qt.MiddleButton
    hoverEnabled: true
    onPressed: wasExpanded = root.expanded
    onPressAndHold: function(mouse) {
        if (mouse.button === Qt.LeftButton)
            root.expanded = !wasExpanded
    }
    onClicked: function(mouse) {
        if (mouse.button === Qt.MiddleButton)
            root.openPanel(Plasmoid.configuration.middleClickPanel)
        else if (Plasmoid.configuration.tapAction === "popup" || wasExpanded)
            root.expanded = !wasExpanded
        else
            root.toggle()
    }

    Rectangle {
        anchors.fill: parent
        anchors.margins: 1
        radius: Kirigami.Units.cornerRadius
        color: Qt.alpha(compact.containsMouse || root.expanded ? Kirigami.Theme.highlightColor : Kirigami.Theme.textColor,
                        compact.pressed ? 0.26 : compact.containsMouse || root.expanded ? 0.12 : 0)
        Behavior on color { ColorAnimation { duration: Kirigami.Units.shortDuration } }
    }

    Ripple {
        anchors.centerIn: parent
        width: Math.min(compact.width, compact.height)
        height: width
    }

    Kirigami.Icon {
        id: icon
        anchors.centerIn: parent
        anchors.verticalCenterOffset: indicator.visible ? -Math.round(indicator.height) : 0
        width: compact.iconSize
        height: width
        source: root.status === "shown" ? "input-keyboard-virtual-on" : "input-keyboard-virtual-off"
        fallback: "input-keyboard-virtual"
        active: compact.containsMouse
        opacity: root.ready || root.status === "checking" ? 1 : 0.6
        scale: compact.pressed ? 0.86 : 1
        Behavior on scale { NumberAnimation { duration: Kirigami.Units.shortDuration; easing.type: Easing.OutCubic } }
        Behavior on anchors.verticalCenterOffset { NumberAnimation { duration: Kirigami.Units.shortDuration; easing.type: Easing.OutCubic } }
    }

    Rectangle {
        id: indicator
        visible: Plasmoid.configuration.showVisibleIndicator
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: icon.bottom
        anchors.topMargin: Math.max(1, Math.round(compact.iconSize * 0.06))
        height: Math.max(2, Math.round(compact.iconSize * 0.1))
        radius: height / 2
        width: root.status === "shown" ? Math.round(compact.iconSize * 0.7) : 0
        opacity: root.status === "shown" ? 1 : 0
        color: Kirigami.Theme.highlightColor
        Behavior on width { NumberAnimation { duration: Kirigami.Units.longDuration; easing.type: Easing.OutCubic } }
        Behavior on opacity { NumberAnimation { duration: Kirigami.Units.longDuration; easing.type: Easing.OutCubic } }
    }

    Rectangle {
        visible: !root.ready && root.status !== "checking"
        anchors.right: icon.right
        anchors.bottom: icon.bottom
        anchors.margins: -Math.round(width * 0.2)
        width: Math.max(6, Math.round(compact.iconSize * 0.34))
        height: width
        radius: width / 2
        color: root.statusColor
        border.width: Math.max(1, Math.round(width * 0.18))
        border.color: Kirigami.Theme.backgroundColor
    }
}
