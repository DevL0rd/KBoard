pragma ComponentBehavior: Bound

import QtQuick
import org.devl0rd.kboard.ui

QtObject {
    id: modifiers

    property string shiftState: "off"
    property bool autoShift: false
    property var sticky: ({ ctrl: "off", alt: "off", super: "off" })

    readonly property bool shifted: shiftState !== "off"
    readonly property int stickyMask: ["ctrl", "alt", "super"].reduce((mask, name) => sticky[name] !== "off" ? mask | KeyNames.modifierMask(name) : mask, 0)
    readonly property int doubleTapWindow: 350

    property double lastShiftTap: 0
    property var lastStickyTap: ({})
    property bool shiftFromAuto: false

    function stateOf(name) {
        return sticky[name] ?? "off"
    }

    function tapShift() {
        const now = Date.now()
        const quick = now - lastShiftTap < doubleTapWindow
        lastShiftTap = now
        shiftFromAuto = false
        if (shiftState === "off")
            shiftState = "on"
        else if (shiftState === "on" && quick)
            shiftState = "locked"
        else
            shiftState = "off"
    }

    function lockShift() {
        shiftFromAuto = false
        shiftState = "locked"
    }

    function tapSticky(name) {
        const now = Date.now()
        const quick = now - (lastStickyTap[name] ?? 0) < doubleTapWindow
        lastStickyTap[name] = now
        const current = stateOf(name)
        const next = current === "off" ? "on" : current === "on" && quick ? "locked" : "off"
        sticky = Object.assign({}, sticky, { [name]: next })
    }

    function consume() {
        if (shiftState === "on")
            shiftState = "off"
        shiftFromAuto = false
        const released = {}
        for (const name in sticky)
            released[name] = sticky[name] === "on" ? "off" : sticky[name]
        sticky = released
    }

    function reset() {
        shiftState = autoShift ? "on" : "off"
        shiftFromAuto = autoShift
        sticky = { ctrl: "off", alt: "off", super: "off" }
    }

    onAutoShiftChanged: {
        if (autoShift && shiftState === "off") {
            shiftState = "on"
            shiftFromAuto = true
        } else if (!autoShift && shiftFromAuto && shiftState === "on") {
            shiftState = "off"
            shiftFromAuto = false
        }
    }
}
