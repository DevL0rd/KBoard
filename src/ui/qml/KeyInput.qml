pragma ComponentBehavior: Bound

import QtQuick
import org.devl0rd.kboard.ui

QtObject {
    id: input

    required property var view
    required property KeyboardSurface surface
    required property AccentPopup popup
    required property GlideTrail trail
    required property KeyModifiers modifiers

    readonly property var pageKeys: ({
        symbols: key => view.pageId = key.page,
        letters: () => view.pageId = "letters",
        fn: () => view.functionRow = !view.functionRow,
        shift: () => modifiers.tapShift(),
        modifier: key => modifiers.tapSticky(key.modifier),
        key: key => sendNamed(key.key),
        arrow: key => sendNamed(key.key),
        char: key => sendText(modifiers.shifted ? key.shiftOutput : key.output),
        space: key => action("space", key),
        backspace: key => action("backspace", key),
        enter: key => action("enter", key),
        emoji: key => action("panel:emoji", key),
        globe: key => action("globe", key),
        hide: key => action("hide", key)
    })

    function action(name, key) {
        view.actionKey(name, key ?? {})
    }

    function activate(key) {
        const handler = pageKeys[key.type]
        if (handler)
            handler(key)
        else if (key.action)
            action(key.action, key)
    }

    function sendText(text) {
        const code = modifiers.stickyMask ? KeyNames.evdev(text.toLowerCase()) : 0
        if (code) {
            view.shortcutKey(code, modifiers.stickyMask | (KeyNames.needsShift(text) ? KeyNames.modifierMask("shift") : 0))
        } else {
            view.textKey(text)
        }
        modifiers.consume()
    }

    function sendNamed(name) {
        view.shortcutKey(KeyNames.evdev(name), modifiers.stickyMask | (modifiers.shifted ? KeyNames.modifierMask("shift") : 0))
        modifiers.consume()
    }

    function activateItem(item) {
        if (!item)
            return
        if (item.action)
            action(item.action, item)
        else
            sendText(item.output)
    }

    function recordPoint(index, x, y) {
        view.pressPoints = Object.assign({}, view.pressPoints, { [index]: { x: x, y: y } })
    }

    function openPopup(touchId, index, items) {
        const rect = surface.keyRect(index)
        popup.cellWidth = items.some(item => item.label.length > 3) ? Math.max(rect.width * 1.6, rect.height * 2.6) : Math.min(rect.width * 0.92, rect.height * 0.9)
        popup.cellHeight = rect.height * 1.05
        popup.maxColumns = items.some(item => item.label.length > 3) ? 3 : 8
        popup.show(items, rect, index, touchId)
        surface.gestures.setTouchMode(touchId, "popup")
        view.popupOpened()
    }

    function languageItems() {
        return view.layoutIds.map(id => ({ label: Layouts.layout(id).name ?? id, output: "", action: "layout:" + id }))
    }

    function longPress(touchId, index) {
        const key = surface.keyData(index)
        if (key.type === "shift") {
            modifiers.lockShift()
            surface.gestures.setTouchMode(touchId, "consumed")
        } else if (key.type === "space" && view.multipleLayouts) {
            openPopup(touchId, index, languageItems())
        } else {
            const items = modifiers.shifted ? key.shiftLongPress : key.longPress
            if (items && items.length > 0)
                openPopup(touchId, index, items)
        }
    }

    function released(touchId, index, mode) {
        if (mode === "tap") {
            activate(surface.keyData(index))
        } else if (mode === "popup" && popup.touchId === touchId) {
            const item = popup.items[popup.selected]
            popup.close()
            activateItem(item)
        } else if (popup.touchId === touchId) {
            popup.close()
        }
    }

    property Connections gestures: Connections {
        target: input.surface.gestures

        function onKeyPressed(touchId, index, x, y) {
            input.recordPoint(index, x, y)
            input.view.keyPressed(input.surface.keyData(index))
        }

        function onPressMoved(touchId, from, to) {
            const rect = input.surface.keyRect(to)
            input.recordPoint(to, rect.x + rect.width / 2, rect.y + rect.height / 2)
        }

        function onKeyReleased(touchId, index, x, y, mode) {
            input.released(touchId, index, mode)
        }

        function onKeyLongPressed(touchId, index) {
            input.longPress(touchId, index)
        }

        function onKeyRepeated(touchId, index, count) {
            const key = input.surface.keyData(index)
            input.view.keyPulsed(index)
            if (key.type === "backspace")
                input.view.backspaceRepeated(count)
            else
                input.activate(key)
        }

        function onTouchMoved(touchId, x, y) {
            if (input.popup.touchId === touchId)
                input.popup.selectAt(x, y)
        }

        function onGlideStarted(touchId, x, y) {
            input.trail.begin(x, y)
            input.view.glideStarted()
        }

        function onGlideMoved(x, y) {
            input.trail.addPoint(x, y)
        }

        function onGlideFinished(points) {
            input.trail.finish()
            input.view.glideFinished(points)
        }

        function onGlideCancelled() {
            input.trail.clear()
        }

        function onCursorSwipe(steps) {
            input.view.cursorSwipe(steps)
        }

        function onCursorSwipeFinished() {
            input.view.cursorSwipeFinished()
        }

        function onDeleteSwipe(words) {
            input.view.deleteSwipe(words)
        }

        function onDeleteSwipeFinished(words) {
            input.view.deleteSwipeFinished(words)
        }
    }
}
