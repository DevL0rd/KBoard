import QtCore
import QtQuick
import org.kde.kirigami as Kirigami
import org.kde.plasma.plasmoid
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.plasma5support as P5Support
import org.kde.plasma.workspace.dbus as DBus

PlasmoidItem {
    id: root

    readonly property string service: "org.devl0rd.KBoard"
    readonly property string home: StandardPaths.writableLocation(StandardPaths.HomeLocation).toString().replace("file://", "")
    readonly property string switchCommand: home + "/.local/bin/kboard-input-method"
    readonly property string inputMethodCommand: "kreadconfig6 --file kwinrc --group Wayland --key InputMethod"
    readonly property string visibleWatch: "busctl --user wait org.devl0rd.KBoard /KBoard org.devl0rd.KBoard EnabledChanged"
    readonly property string panelWatch: "busctl --user wait org.devl0rd.KBoard /KBoard org.devl0rd.KBoard PanelChanged"
    readonly property bool inPanel: Plasmoid.formFactor === PlasmaCore.Types.Horizontal || Plasmoid.formFactor === PlasmaCore.Types.Vertical

    readonly property bool running: watcher.registered
    property bool keyboardVisible: false
    property string currentPanel: "keys"
    property string inputMethod: ""
    property bool inputMethodRead: false
    readonly property bool activeKeyboard: /(^|\/)org\.devl0rd\.kboard\.desktop$/.test(inputMethod)
    property bool switching: false
    property string errorText: ""
    property string watchError: ""

    readonly property string status: !inputMethodRead ? "checking"
        : errorText !== "" || watchError !== "" ? "error"
        : !activeKeyboard ? "inactive"
        : !running ? "stopped"
        : keyboardVisible ? "shown" : "hidden"
    readonly property bool ready: status === "shown" || status === "hidden"
    readonly property color statusColor: status === "shown" ? Kirigami.Theme.positiveTextColor
        : status === "hidden" ? Kirigami.Theme.highlightColor
        : status === "checking" ? Kirigami.Theme.disabledTextColor
        : status === "stopped" ? Kirigami.Theme.neutralTextColor
        : Kirigami.Theme.negativeTextColor
    readonly property string statusText: {
        switch (status) {
        case "checking": return i18n("Checking KWin's on-screen keyboard…")
        case "error": return watchError !== "" ? watchError : errorText
        case "inactive": return i18n("KBoard is not the active keyboard")
        case "stopped": return i18n("KBoard is not running")
        case "shown": return currentPanel === "keys" ? i18n("Keyboard is showing") : i18n("Showing %1", panelName(currentPanel))
        default: return i18n("Keyboard is hidden")
        }
    }
    readonly property string previousInputMethodName: {
        const name = inputMethod.split("/").pop().replace(/\.desktop$/, "")
        return name === "" ? i18n("no on-screen keyboard") : name
    }

    signal triggered()

    Plasmoid.icon: status === "shown" ? "input-keyboard-virtual-on" : "input-keyboard-virtual-off"
    Plasmoid.title: i18n("KBoard")
    preferredRepresentation: inPanel ? compactRepresentation : fullRepresentation
    switchWidth: Kirigami.Units.gridUnit * 14
    switchHeight: Kirigami.Units.gridUnit * 12
    toolTipMainText: i18n("KBoard")
    toolTipSubText: ready ? i18n("%1\nClick to %2 · Middle-click for %3", statusText, keyboardVisible ? i18n("turn off") : i18n("turn on"), panelName(Plasmoid.configuration.middleClickPanel)) : statusText

    Plasmoid.contextualActions: [
        PlasmaCore.Action {
            text: root.keyboardVisible ? i18n("Turn Keyboard Off") : i18n("Turn Keyboard On")
            icon.name: root.keyboardVisible ? "input-keyboard-virtual-hide" : "input-keyboard-virtual-show"
            enabled: root.ready
            onTriggered: root.toggle()
        },
        PlasmaCore.Action {
            text: i18n("Quick Actions…")
            icon.name: "view-list-icons"
            onTriggered: root.expanded = true
        },
        PlasmaCore.Action {
            text: i18n("KBoard Settings…")
            icon.name: "configure"
            enabled: root.running
            onTriggered: root.openSettings("")
        }
    ]

    function panelName(name) {
        switch (name) {
        case "voice": return i18n("voice typing")
        case "emoji": return i18n("emoji")
        case "gif": return i18n("GIFs")
        case "clipboard": return i18n("clipboard")
        case "edit": return i18n("text editing")
        case "symbols": return i18n("symbols")
        case "keys": return i18n("keys")
        default: return i18n("nothing")
        }
    }

    function call(member, args, onValue) {
        const strings = args || []
        DBus.SessionBus.asyncCall({
            service: root.service,
            path: "/KBoard",
            iface: root.service,
            member: member,
            arguments: strings.map(value => new DBus.string(value)),
            signature: strings.length ? "(" + "s".repeat(strings.length) + ")" : ""
        }, reply => {
            root.errorText = ""
            if (onValue)
                onValue(reply.value)
        }, reply => {
            root.errorText = i18n("KBoard did not answer %1: %2", member, reply.error.message)
        })
    }

    function sync() {
        if (!running)
            return
        call("IsEnabled", [], value => root.keyboardVisible = value)
        call("CurrentPanel", [], value => root.currentPanel = value)
    }

    function toggle() {
        if (!ready) {
            expanded = true
            return
        }
        triggered()
        call(root.keyboardVisible ? "Disable" : "Enable")
    }

    function show() {
        triggered()
        call("Show")
    }

    function hide() {
        triggered()
        call("Hide")
    }

    function openPanel(name) {
        if (name === "")
            return
        if (!ready) {
            expanded = true
            return
        }
        triggered()
        call("OpenPanel", [name])
    }

    function openSettings(page) {
        call("OpenSettings", [page])
        expanded = false
    }

    function retry() {
        errorText = ""
        if (watchError !== "") {
            watchError = ""
            arm(visibleWatch)
            arm(panelWatch)
        }
        readInputMethod()
        sync()
    }

    function readInputMethod() {
        executable.connectSource(inputMethodCommand)
    }

    function useKBoard() {
        switching = true
        errorText = ""
        executable.connectSource(switchCommand + (activeKeyboard ? " restart" : " enable"))
    }

    function arm(command) {
        Qt.callLater(() => executable.connectSource(command))
    }

    function handle(source, data) {
        const code = data["exit code"]
        const stdout = String(data.stdout || "").trim()
        const stderr = String(data.stderr || "").trim()
        if (source === inputMethodCommand) {
            inputMethod = stdout
            inputMethodRead = true
        } else if (source === visibleWatch || source === panelWatch) {
            if (code !== 0) {
                watchError = i18n("Could not watch KBoard over D-Bus: %1", stderr || stdout)
                return
            }
            if (source === visibleWatch)
                keyboardVisible = stdout === "b true"
            else
                currentPanel = stdout.replace(/^s "(.*)"$/, "$1")
            arm(source)
            sync()
        } else if (source.startsWith(switchCommand)) {
            switching = false
            if (code !== 0)
                errorText = stdout || stderr || i18n("Switching the on-screen keyboard failed")
            readInputMethod()
        }
    }

    onExpandedChanged: function(isExpanded) {
        if (isExpanded) {
            readInputMethod()
            sync()
        }
    }

    Component.onCompleted: {
        readInputMethod()
        arm(visibleWatch)
        arm(panelWatch)
    }

    DBus.DBusServiceWatcher {
        id: watcher
        busType: DBus.BusType.Session
        watchedService: root.service
        onRegisteredChanged: {
            if (registered) {
                root.errorText = ""
                root.sync()
            } else {
                root.keyboardVisible = false
            }
            root.readInputMethod()
        }
    }

    P5Support.DataSource {
        id: executable
        engine: "executable"
        connectedSources: []
        onNewData: function(source, data) {
            disconnectSource(source)
            root.handle(source, data)
        }
    }

    compactRepresentation: CompactView {}
    fullRepresentation: FullView {}
}
