pragma ComponentBehavior: Bound

import QtQuick
import org.devl0rd.kboard
import org.devl0rd.kboard.config
import org.devl0rd.kboard.platform
import org.devl0rd.kboard.gamepad
import org.devl0rd.kboard.sound
import org.devl0rd.kboard.ui

PanelWindow {
    id: window

    readonly property real baseHeight: Math.round(screenHeight * (portrait ? Settings.heightPortrait : Settings.heightLandscape))
    property bool shown: false

    function visibleCells(item, found) {
        if (!item || !item.visible || found.length >= 8)
            return found
        if (typeof item.base === "string" && item.text) {
            const point = item.mapToItem(keyboard, 0, 0)
            if (point.y >= keyboard.panelTop)
                found.push({ text: item.text, x: point.x, y: point.y, width: item.width, height: item.height })
        }
        for (const child of item.children)
            visibleCells(child, found)
        return found
    }

    function present() {
        if (keyboard.rules.neverShow) {
            KeyboardService.Hide()
            return
        }
        if (shown)
            return
        shown = true
        keyboard.slide = 1
        window.visible = true
        KeySound.play("open")
        slideIn.restart()
    }

    function dismiss() {
        if (!shown)
            return
        shown = false
        keyboard.panel = "keys"
        KeySound.play("close")
        slideOut.restart()
    }

    width: screenWidth
    height: Math.ceil(keyboard.height)
    visible: false
    onVisibleChanged: KeyboardService.visible = visible
    interactiveRects: keyboard.interactiveRects
    blurRect: interactiveRects.length === 1 ? interactiveRects[0] : Qt.rect(0, keyboard.panelTop, width, keyboard.targetHeight)
    blurEnabled: Settings.blurEnabled && interactiveRects.length === 1

    Binding {
        target: KeySound
        property: "active"
        value: KeyboardService.visible
    }

    Component.onCompleted: {
        if (!initPanel()) {
            console.error("KBoard must be launched by KWin as the virtual keyboard")
            Qt.exit(1)
        }
    }

    Keyboard {
        id: keyboard

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        baseHeight: window.baseHeight
        screenHeight: window.screenHeight
        shown: window.shown
        panel: KeyboardService.panel
        application: KeyboardService.activeApp
        onPanelChanged: KeyboardService.panel = panel
        onHideRequested: KeyboardService.Hide()
        onSettingsRequested: KeyboardService.OpenSettings("")
    }

    NumberAnimation {
        id: slideIn

        target: keyboard
        property: "slide"
        to: 0
        duration: Theme.duration(280)
        easing.type: Easing.OutBack
        easing.overshoot: 0.8
    }

    SequentialAnimation {
        id: slideOut

        NumberAnimation {
            target: keyboard
            property: "slide"
            to: 1
            duration: Theme.duration(200)
            easing.type: Easing.InCubic
        }
        ScriptAction {
            script: {
                if (!window.shown)
                    window.visible = false
            }
        }
    }

    Connections {
        target: InputContext

        function onActivated() {
            if (Settings.showOnFocus)
                window.present()
        }

        function onDeactivated() {
            window.dismiss()
        }
    }

    Connections {
        target: KeyboardService

        function onShowRequested() {
            window.present()
        }

        function onHideRequested() {
            window.dismiss()
        }

        function onPanelChanged() {
            keyboard.panel = KeyboardService.panel
        }

        function onKeyMapRequested() {
            const description = keyboard.describe()
            description.application = KeyboardService.activeApp
            description.cells = window.visibleCells(keyboard.currentPanel, [])
            description.window = { width: window.width, height: window.height, visible: window.visible, screenWidth: window.screenWidth, screenHeight: window.screenHeight }
            KeyboardService.keyMap = JSON.stringify(description)
        }
    }

    Connections {
        target: Gamepad
        enabled: Settings.controllerEnabled

        function onChordActivated() {
            KeyboardService.Show()
        }
    }
}
