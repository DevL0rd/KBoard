pragma ComponentBehavior: Bound

import QtQuick
import org.devl0rd.kboard.config

Item {
    id: cap

    required property int index
    required property var keyData
    required property string keyType
    required property real keyX
    required property real keyY
    required property real keyWidth
    required property real keyHeight
    required property bool pressed
    required property var keyboard

    readonly property bool special: keyData.special === true
    readonly property bool accentKey: keyType === "enter"
    readonly property bool latched: keyType === "modifier" && keyboard.modifierState(keyData.modifier) !== "off"
        || keyType === "fn" && keyboard.functionRow
    readonly property string iconName: ({
        shift: "shift", backspace: "backspace", enter: "enter", globe: "globe", emoji: "emoji", arrow: keyData.icon, hide: "hide"
    })[keyType] ?? (keyData.icon === "tab" || keyData.icon === "super" ? keyData.icon : "")
    readonly property string text: {
        if (keyType === "space")
            return keyboard.spaceLabel
        if (iconName !== "")
            return ""
        return keyboard.shifted && keyType === "char" ? keyData.shiftLabel : keyData.label
    }
    readonly property bool bigLabel: keyType === "char" && text.length === 1
    readonly property bool hasLongPress: (keyData.longPress ?? []).length > 0 || keyType === "space" && keyboard.multipleLayouts

    x: keyX
    y: keyY
    width: keyWidth
    height: keyHeight

    Rectangle {
        anchors.fill: body
        anchors.topMargin: Math.max(1, Math.round(cap.height * 0.03))
        anchors.bottomMargin: -anchors.topMargin
        radius: body.radius
        color: Theme.keyShadow
        visible: Theme.raised
        scale: body.scale
        opacity: 0.9
    }

    Rectangle {
        id: body

        anchors.fill: parent
        radius: Math.min(Theme.radius, height / 2)
        color: {
            if (cap.accentKey)
                return cap.pressed ? Qt.lighter(Theme.accent, Theme.dark ? 1.25 : 0.88) : Theme.accent
            if (cap.latched)
                return Theme.accentSoft
            if (cap.pressed)
                return cap.special ? Theme.specialKeyPressed : Theme.keyPressed
            return cap.special ? Theme.specialKey : Theme.key
        }
        border.width: Theme.bordered || Theme.glass ? 1 : 0
        border.color: cap.latched ? Theme.accent : Theme.keyBorder
        clip: true

        Behavior on color {
            ColorAnimation {
                duration: Theme.duration(cap.pressed ? 50 : 160)
                easing.type: Easing.OutCubic
            }
        }

        Rectangle {
            anchors.fill: parent
            radius: parent.radius
            visible: Theme.glass || (Theme.raised && Theme.dark)
            gradient: Gradient {
                GradientStop { position: 0; color: Theme.glass ? Qt.alpha("#ffffff", Theme.dark ? 0.08 : 0.35) : Theme.keySheen }
                GradientStop { position: 0.55; color: "transparent" }
            }
        }

        KeyRipple {
            anchors.fill: parent
            pressed: cap.pressed
            origin: {
                const point = cap.keyboard.pressPoints[cap.index]
                return point ? Qt.point(point.x - cap.keyX, point.y - cap.keyY) : Qt.point(width / 2, height / 2)
            }
            radius: parent.radius
            enabled: Settings.ripple && !cap.accentKey
        }
    }

    Text {
        anchors.centerIn: parent
        anchors.verticalCenterOffset: cap.keyData.sublabel ? -cap.height * 0.1 : 0
        text: cap.text
        color: cap.accentKey ? Theme.accentText : cap.latched ? Theme.accent : cap.keyType === "space" ? Theme.keyHint : Theme.keyText
        font.family: Theme.font.family
        font.pixelSize: Math.max(8, Math.round((cap.bigLabel ? Math.min(cap.height * 0.42, cap.width * 0.56) : Math.min(cap.height * 0.27, cap.width * 0.3)) * Theme.labelScale))
        font.weight: cap.bigLabel ? Font.Normal : Font.Medium
        scale: body.scale
    }

    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: cap.height * 0.1
        visible: text !== ""
        text: cap.keyData.sublabel ?? ""
        color: Theme.keyHint
        font.pixelSize: Math.max(7, Math.round(cap.height * 0.16 * Theme.labelScale))
        font.letterSpacing: 1
    }

    KeyIcon {
        anchors.centerIn: parent
        width: Math.min(cap.height * 0.5, cap.width * 0.6)
        height: width
        visible: cap.iconName !== ""
        name: cap.iconName
        shiftState: cap.keyboard.shiftState
        color: cap.accentKey ? Theme.accentText : Theme.keyText
        scale: body.scale
    }

    Text {
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.topMargin: Math.round(cap.height * 0.07)
        anchors.rightMargin: Math.round(Math.min(cap.width, cap.height) * 0.12)
        visible: Settings.showKeyHints && text !== "" && cap.keyType === "char"
        text: cap.keyboard.shifted ? (cap.keyData.hint ?? "").toUpperCase() : (cap.keyData.hint ?? "")
        color: Theme.keyHint
        font.pixelSize: Math.max(7, Math.round(cap.height * 0.19 * Theme.labelScale))
        scale: body.scale

        LongPressRing {
            anchors.centerIn: parent
            width: Math.max(parent.width, parent.height) + cap.height * 0.14
            height: width
            running: cap.pressed && cap.hasLongPress && cap.keyType === "char"
            duration: Settings.longPressDelay
        }
    }

    NumberAnimation {
        id: pressAnimation

        target: body
        property: "scale"
        to: 0.94
        duration: Theme.duration(70)
        easing.type: Easing.OutCubic
    }

    NumberAnimation {
        id: releaseAnimation

        target: body
        property: "scale"
        to: 1
        duration: Theme.duration(300)
        easing.type: Easing.OutBack
        easing.overshoot: 3
    }

    onPressedChanged: {
        if (pressed) {
            releaseAnimation.stop()
            pressAnimation.restart()
        } else {
            pressAnimation.stop()
            releaseAnimation.restart()
        }
    }

    Connections {
        target: cap.keyboard

        function onKeyPulsed(index) {
            if (index === cap.index)
                pulseAnimation.restart()
        }
    }

    SequentialAnimation {
        id: pulseAnimation

        NumberAnimation { target: body; property: "scale"; to: 0.88; duration: Theme.duration(40); easing.type: Easing.OutCubic }
        NumberAnimation { target: body; property: "scale"; to: cap.pressed ? 0.94 : 1; duration: Theme.duration(120); easing.type: Easing.OutBack }
    }
}
