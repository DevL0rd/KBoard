import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

Item {
    id: clear

    property bool armed: false
    property real remaining: 1
    property string confirmText

    signal confirmed()

    function tap() {
        if (armed) {
            drain.stop()
            armed = false
            confirmed()
        } else {
            armed = true
            drain.restart()
        }
    }

    Layout.preferredHeight: Kirigami.Units.gridUnit * 2
    Layout.preferredWidth: armed ? row.implicitWidth + Kirigami.Units.largeSpacing * 2.5 : Kirigami.Units.gridUnit * 2
    opacity: enabled ? 1 : 0.4
    clip: true
    onEnabledChanged: {
        if (!enabled) {
            drain.stop()
            armed = false
        }
    }

    Behavior on Layout.preferredWidth { NumberAnimation { duration: ClipboardStyle.duration(220); easing.type: Easing.OutCubic } }

    Rectangle {
        anchors.fill: parent
        radius: height / 2
        color: clear.armed ? Qt.alpha(Kirigami.Theme.negativeTextColor, tapHandler.pressed ? 0.3 : 0.18)
            : tapHandler.pressed ? Qt.alpha(Kirigami.Theme.textColor, 0.16)
            : hover.hovered ? Qt.alpha(Kirigami.Theme.textColor, 0.08) : "transparent"
        Behavior on color { ColorAnimation { duration: ClipboardStyle.duration(160) } }
    }

    Rectangle {
        visible: clear.armed
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottomMargin: 3
        height: 2
        radius: 1
        width: (parent.width - parent.height) * clear.remaining
        color: Kirigami.Theme.negativeTextColor
    }

    Row {
        id: row
        anchors.centerIn: parent
        spacing: Kirigami.Units.smallSpacing

        Kirigami.Icon {
            width: Kirigami.Units.iconSizes.smallMedium
            height: width
            anchors.verticalCenter: parent.verticalCenter
            source: clear.armed ? "dialog-warning" : "edit-clear-history"
            color: clear.armed ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.textColor
            isMask: true
        }

        Text {
            visible: clear.armed
            anchors.verticalCenter: parent.verticalCenter
            text: clear.confirmText
            color: Kirigami.Theme.negativeTextColor
            font.weight: Font.DemiBold
        }
    }

    HoverHandler { id: hover }

    TapHandler {
        id: tapHandler
        onTapped: clear.tap()
    }

    NumberAnimation {
        id: drain
        target: clear
        property: "remaining"
        from: 1
        to: 0
        duration: 4000
        onFinished: clear.armed = false
    }

    QQC2.ToolTip.visible: hover.hovered && !clear.armed
    QQC2.ToolTip.text: i18n("Clear history")
    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
}
