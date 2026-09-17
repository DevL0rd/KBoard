pragma ComponentBehavior: Bound

import QtQuick
import org.devl0rd.kboard.config
import org.devl0rd.kboard.platform
import org.devl0rd.kboard.gamepad

QtObject {
    id: navigation

    required property var keyboard
    required property KeyboardView keys
    property var panel
    property bool active: false
    property bool shown: false
    property int stickDirection: 0

    readonly property bool enabled: Settings.controllerEnabled && shown
    readonly property var panelOrder: ["keys", "emoji", "gif", "clipboard", "voice", "edit"]
    readonly property var buttons: ({
        a: () => activate(),
        b: () => keys.actionKey("backspace", {}),
        x: () => keys.actionKey("space", {}),
        y: () => keys.modifiers.tapShift(),
        leftshoulder: () => InputContext.moveCursor(-1),
        rightshoulder: () => InputContext.moveCursor(1),
        lefttrigger: () => cyclePanel(-1),
        righttrigger: () => cyclePanel(1),
        start: () => keys.actionKey("enter", {}),
    })
    readonly property var clipboardButtons: ({
        y: "togglePinCurrent",
        x: "removeCurrent"
    })

    function onKeys() {
        return keyboard.panel === "keys"
    }

    function callPanel(method, ...args) {
        if (panel && typeof panel[method] === "function") {
            panel[method](...args)
        } else {
            console.warn("KBoard controller: panel", keyboard.panel, "does not implement", method)
        }
    }

    function ensureFocus() {
        if (keys.focusIndex < 0) {
            const start = keys.surface.indexOf("g")
            keys.focusIndex = start >= 0 ? start : 0
        }
    }

    function navigate(dx, dy) {
        active = true
        if (!onKeys()) {
            callPanel("moveFocus", dx, dy)
            return
        }
        if (keys.popup.open && keys.popup.touchId === -3) {
            keys.popup.selected = Math.max(0, Math.min(keys.popup.items.length - 1, keys.popup.selected + dx - dy * keys.popup.columns))
            return
        }
        const had = keys.focusIndex >= 0
        ensureFocus()
        if (had)
            keys.focusIndex = keys.surface.neighbor(keys.focusIndex, dx, dy)
    }

    function activate() {
        active = true
        if (!onKeys()) {
            callPanel("activateCurrent")
            return
        }
        ensureFocus()
        keys.activateIndex(keys.focusIndex)
        if (Settings.controllerRumble)
            Gamepad.rumble(0.3, 20)
    }

    function cyclePanel(step) {
        const index = panelOrder.indexOf(keyboard.panel)
        keyboard.panel = panelOrder[(index + step + panelOrder.length) % panelOrder.length]
    }

    function holdA() {
        if (onKeys() && keys.focusIndex >= 0)
            keys.openLongPressAt(keys.focusIndex, -3)
    }

    property Connections gamepad: Connections {
        target: Gamepad
        enabled: navigation.enabled

        function onNavigate(dx, dy) {
            navigation.navigate(dx, dy)
        }

        function onButtonPressed(button) {
            if (button === "a" && navigation.onKeys()) {
                navigation.ensureFocus()
                navigation.active = true
                navigation.hold.restart()
                return
            }
            const panelMethod = navigation.keyboard.panel === "clipboard" ? navigation.clipboardButtons[button] : undefined
            if (panelMethod) {
                navigation.callPanel(panelMethod)
                return
            }
            const handler = navigation.buttons[button]
            if (handler)
                handler()
        }

        function onButtonReleased(button) {
            if (button !== "a" || !navigation.onKeys())
                return
            if (navigation.hold.running) {
                navigation.hold.stop()
                navigation.activate()
            } else if (navigation.keys.popup.touchId === -3) {
                navigation.keys.commitPopup()
            }
        }

        function onButtonTapped(button) {
            if (button === "back")
                navigation.keyboard.hideRequested()
        }

        function onRightStickChanged() {
            const x = Gamepad.rightStick.x
            const direction = x > 0.6 ? 1 : x < -0.6 ? -1 : 0
            if (direction !== navigation.stickDirection) {
                navigation.stickDirection = direction
                if (direction !== 0)
                    InputContext.moveCursor(direction)
            }
        }
    }

    property Timer hold: Timer {
        interval: Settings.longPressDelay
        onTriggered: navigation.holdA()
    }

    onEnabledChanged: {
        if (!enabled) {
            active = false
            keys.focusIndex = -1
        }
    }
}
