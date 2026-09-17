pragma ComponentBehavior: Bound

import QtQuick
import org.devl0rd.kboard.ui

QtObject {
    id: demo

    required property var view
    required property KeyboardSurface surface
    required property GlideTrail trail
    required property KeyInput input

    property int pressedIndex: -1
    property var glidePoints: []
    property string glideWord: ""
    property real glideProgress: 0

    readonly property var namedTypes: ["shift", "backspace", "space", "enter", "symbols", "letters", "emoji", "globe"]

    function indexFor(label) {
        return namedTypes.includes(label) ? surface.indexOfType(label) : surface.indexOf(label)
    }

    function press(label) {
        const index = indexFor(label)
        if (index >= 0)
            tap(index)
    }

    function tap(index) {
        if (pressedIndex >= 0)
            releaseTimer.triggered()
        const rect = surface.keyRect(index)
        input.recordPoint(index, rect.x + rect.width / 2, rect.y + rect.height / 2)
        surface.setVisualPressed(index, true)
        view.keyPressed(surface.keyData(index))
        pressedIndex = index
        releaseTimer.restart()
    }

    function glide(word) {
        glidePoints = surface.glidePath(word)
        if (glidePoints.length < 2)
            return
        glideWord = word
        glideProgress = 0
        trail.begin(glidePoints[0].x, glidePoints[0].y)
        glideAnimation.duration = Math.max(450, glidePoints.length * 150)
        glideAnimation.restart()
    }

    function pointAt(progress) {
        const scaled = progress * (glidePoints.length - 1)
        const segment = Math.min(glidePoints.length - 2, Math.floor(scaled))
        const t = scaled - segment
        const from = glidePoints[segment]
        const to = glidePoints[segment + 1]
        return Qt.point(from.x + (to.x - from.x) * t, from.y + (to.y - from.y) * t)
    }

    function finishGlide() {
        trail.finish()
        if (view.preview)
            view.previewText += (view.previewText.length > 0 && !view.previewText.endsWith(" ") ? " " : "") + glideWord
        else
            view.glideFinished(glidePoints)
    }

    onGlideProgressChanged: {
        if (glidePoints.length >= 2) {
            const point = pointAt(glideProgress)
            trail.addPoint(point.x, point.y)
        }
    }

    property Timer releaseTimer: Timer {
        interval: 110

        onTriggered: {
            const index = demo.pressedIndex
            demo.pressedIndex = -1
            demo.releaseTimer.stop()
            if (index < 0)
                return
            demo.surface.setVisualPressed(index, false)
            demo.input.activate(demo.surface.keyData(index))
        }
    }

    property NumberAnimation glideAnimation: NumberAnimation {
        target: demo
        property: "glideProgress"
        from: 0
        to: 1
        easing.type: Easing.InOutSine
        onFinished: demo.finishGlide()
    }

    property Connections previewTyping: Connections {
        target: demo.view
        enabled: demo.view.preview

        function onTextKey(text) {
            demo.view.previewText += text
        }

        function onActionKey(action, key) {
            if (action === "space")
                demo.view.previewText += " "
            else if (action === "backspace")
                demo.view.previewText = demo.view.previewText.slice(0, -1)
            else if (action === "enter")
                demo.view.previewText += "\n"
        }
    }
}
