pragma ComponentBehavior: Bound

import QtQuick
import org.devl0rd.kboard.config
import org.devl0rd.kboard.ui

Item {
    id: view

    property string layoutId: Settings.activeLayout
    property var layoutIds: Settings.layouts
    property string pageId: "letters"
    property string variant: ""
    property bool numberRow: Settings.showNumberRow
    property bool desktopRow: Settings.showDesktopRow
    property bool functionRow: false
    property bool split: false
    property real splitGap: 0
    property bool preview: false
    property bool glideEnabled: Settings.glideTyping
    property bool glideTrail: Settings.glideTrail
    property int focusIndex: -1
    property var pressPoints: ({})
    property string previewText: ""

    readonly property alias surface: surface
    readonly property alias modifiers: keyModifiers
    readonly property alias popup: popup
    readonly property string shiftState: keyModifiers.shiftState
    readonly property bool shifted: keyModifiers.shifted
    readonly property bool multipleLayouts: layoutIds.length > 1
    readonly property var layout: Layouts.revision >= 0 ? Layouts.layout(layoutId) : ({})
    readonly property string spaceLabel: multipleLayouts ? layout.name ?? "" : ""
    readonly property int popupKeyIndex: popup.keyIndex
    readonly property var page: Layouts.revision >= 0 ? Layouts.page(layoutId, pageId, {
        numberRow: pageId === "letters" && numberRow,
        desktopRow: desktopRow,
        functionRow: functionRow,
        variant: variant,
        multipleLayouts: multipleLayouts
    }) : ({})
    readonly property bool valid: surface.count > 0
    readonly property var interactiveRects: surface.halfRects

    signal keyPressed(var key)
    signal textKey(string text)
    signal actionKey(string action, var key)
    signal shortcutKey(int code, int modifiers)
    signal backspaceRepeated(int count)
    signal cursorSwipe(int steps)
    signal cursorSwipeFinished
    signal deleteSwipe(int words)
    signal deleteSwipeFinished(int words)
    signal glideStarted
    signal glideFinished(var points)
    signal keyPulsed(int index)
    signal popupOpened

    function modifierState(name) {
        return keyModifiers.stateOf(name)
    }

    function activateIndex(index) {
        demo.tap(index)
    }

    function openLongPressAt(index, touchId) {
        keyInput.longPress(touchId, index)
    }

    function commitPopup() {
        keyInput.released(popup.touchId, popup.keyIndex, "popup")
    }

    function demoPress(label) {
        demo.press(label)
    }

    function demoGlide(word) {
        demo.glide(word)
    }

    function glideKeys() {
        return surface.glideKeys()
    }

    onLayoutIdChanged: pageId = "letters"

    KeyboardSurface {
        id: surface

        anchors.fill: parent
        page: view.page
        gap: Theme.gap
        split: view.split
        splitGap: view.splitGap
        interactive: !view.preview && view.visible
        gestures.longPressDelay: Settings.longPressDelay
        gestures.repeatDelay: Settings.keyRepeatDelay
        gestures.repeatInterval: Settings.keyRepeatInterval
        gestures.glideEnabled: view.glideEnabled
        gestures.cursorSwipeEnabled: Settings.spaceCursorSwipe
        gestures.deleteSwipeEnabled: Settings.backspaceSwipeDelete

        Item {
            id: keyLayer

            anchors.fill: parent

            Repeater {
                model: surface.model

                delegate: KeyCap {
                    keyboard: view
                }
            }
        }

        FocusRing {
            target: view.focusIndex >= 0 && surface.halfRects.length > 0 ? surface.keyRect(view.focusIndex) : Qt.rect(0, 0, 0, 0)
            radius: Math.min(Theme.radius, target.height / 2)
            visible: view.focusIndex >= 0
        }

        GlideTrail {
            id: trail

            anchors.fill: parent
            visible: view.glideTrail
            color: Theme.accent
            lineWidth: Math.max(4, surface.unitWidth * 0.07)
            glowWidth: Math.max(14, surface.unitWidth * 0.26)
            tailDuration: Theme.animated ? 700 : 0
            fadeDuration: Theme.duration(320)
        }

        Repeater {
            model: surface.model

            delegate: KeyBubble {
                keyboard: view
            }
        }

        AccentPopup {
            id: popup
        }
    }

    KeyModifiers {
        id: keyModifiers
    }

    KeyInput {
        id: keyInput

        view: view
        surface: surface
        popup: popup
        trail: trail
        modifiers: keyModifiers
    }

    PreviewDemo {
        id: demo

        view: view
        surface: surface
        trail: trail
        input: keyInput
    }

    onPageChanged: pageTransition.restart()

    ParallelAnimation {
        id: pageTransition

        NumberAnimation {
            target: keyLayer
            property: "opacity"
            from: 0.35
            to: 1
            duration: Theme.duration(180)
            easing.type: Easing.OutCubic
        }
        NumberAnimation {
            target: keyLayer
            property: "x"
            from: surface.unitWidth * 0.12
            to: 0
            duration: Theme.duration(220)
            easing.type: Easing.OutCubic
        }
    }
}
