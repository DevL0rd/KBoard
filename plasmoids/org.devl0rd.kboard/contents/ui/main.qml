pragma ComponentBehavior: Bound

import QtQuick
import org.kde.kirigami as Kirigami
import org.kde.plasma.plasmoid
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.plasma5support as P5Support
import org.kde.plasma.workspace.dbus as DBus

PlasmoidItem {
    id: root

    readonly property string service: "org.devl0rd.KBoard"
    readonly property bool running: watcher.registered
    property bool keyboardEnabled: true

    function setEnabled(enabled) {
        DBus.SessionBus.asyncCall({
            service: root.service,
            path: "/KBoard",
            iface: root.service,
            member: "SetEnabled",
            arguments: [new DBus.bool(enabled)],
            signature: "(b)"
        }, () => root.keyboardEnabled = enabled)
    }

    function refresh() {
        if (!running)
            return
        DBus.SessionBus.asyncCall({
            service: root.service,
            path: "/KBoard",
            iface: root.service,
            member: "IsEnabled"
        }, reply => root.keyboardEnabled = reply.value === true)
    }

    Plasmoid.icon: keyboardEnabled ? "input-keyboard-virtual" : "input-keyboard-virtual-off"
    Plasmoid.title: i18n("KBoard")
    preferredRepresentation: compactRepresentation
    toolTipMainText: i18n("On-screen keyboard")
    toolTipSubText: running
        ? (keyboardEnabled ? i18n("Enabled. Click to turn the keyboard off.") : i18n("Disabled. Click to turn it on and show it."))
        : i18n("KBoard is not running")

    DBus.DBusServiceWatcher {
        id: watcher
        busType: DBus.BusType.Session
        watchedService: root.service
        onRegisteredChanged: root.refresh()
    }

    P5Support.DataSource {
        id: enabledWatch
        engine: "executable"
        connectedSources: root.running ? ["busctl --user wait org.devl0rd.KBoard /KBoard org.devl0rd.KBoard EnabledChanged"] : []
        onNewData: function (source) {
            disconnectSource(source)
            root.refresh()
            if (root.running)
                connectSource(source)
        }
    }

    Component.onCompleted: refresh()

    compactRepresentation: MouseArea {
        id: compact

        readonly property bool vertical: Plasmoid.formFactor === PlasmaCore.Types.Vertical
        readonly property real thickness: vertical ? width : height

        implicitWidth: Kirigami.Units.gridUnit * 1.5
        implicitHeight: Kirigami.Units.gridUnit * 1.5
        hoverEnabled: true
        enabled: root.running
        onClicked: root.setEnabled(!root.keyboardEnabled)

        Rectangle {
            anchors.fill: parent
            anchors.margins: 1
            radius: Kirigami.Units.cornerRadius
            color: Qt.alpha(compact.containsMouse ? Kirigami.Theme.highlightColor : Kirigami.Theme.textColor,
                            compact.pressed ? 0.28 : compact.containsMouse ? 0.14 : 0)
            Behavior on color { ColorAnimation { duration: Kirigami.Units.shortDuration } }
        }

        Kirigami.Icon {
            anchors.centerIn: parent
            width: Math.round(Math.min(compact.width, compact.height) * 0.66)
            height: width
            source: Plasmoid.icon
            opacity: root.running ? (root.keyboardEnabled ? 1 : 0.55) : 0.35
            Behavior on opacity { NumberAnimation { duration: Kirigami.Units.shortDuration } }
        }
    }
}
