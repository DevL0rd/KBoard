pragma Singleton

import QtQuick
import org.kde.kirigami as Kirigami
import org.devl0rd.kboard.config

QtObject {
    id: theme

    Kirigami.Theme.colorSet: Kirigami.Theme.Window
    Kirigami.Theme.inherit: false

    readonly property color base: Kirigami.Theme.backgroundColor
    readonly property color text: Kirigami.Theme.textColor
    readonly property color disabledText: Kirigami.Theme.disabledTextColor
    readonly property color systemHighlight: Kirigami.Theme.highlightColor
    readonly property color positive: Kirigami.Theme.positiveTextColor
    readonly property color negative: Kirigami.Theme.negativeTextColor
    readonly property font font: Kirigami.Theme.defaultFont
    readonly property bool dark: Kirigami.ColorUtils.brightnessForColor(base) === Kirigami.ColorUtils.Dark

    readonly property int style: Settings.keyStyle
    readonly property bool flat: style === 0
    readonly property bool bordered: style === 1
    readonly property bool raised: style === 2
    readonly property bool glass: style === 3

    readonly property color accent: Settings.accentMode === 1 ? Settings.accentColor : systemHighlight
    readonly property color accentText: Kirigami.ColorUtils.brightnessForColor(accent) === Kirigami.ColorUtils.Dark ? "#ffffff" : "#121212"
    readonly property color accentSoft: Qt.alpha(accent, dark ? 0.28 : 0.2)

    readonly property real opacity: Settings.backgroundOpacity
    readonly property color panel: Qt.alpha(dark ? mix(base, "#000000", 0.28) : mix(base, text, 0.07), opacity)
    readonly property color panelSolid: dark ? mix(base, "#000000", 0.28) : mix(base, text, 0.07)
    readonly property color panelEdge: Qt.alpha(text, dark ? 0.08 : 0.1)

    readonly property color key: {
        if (glass)
            return dark ? Qt.alpha("#ffffff", 0.1) : Qt.alpha("#ffffff", 0.6)
        if (bordered)
            return Qt.alpha(text, dark ? 0.03 : 0.02)
        return dark ? mix(base, text, 0.13) : mix(base, "#ffffff", 0.88)
    }
    readonly property color specialKey: {
        if (glass)
            return dark ? Qt.alpha("#ffffff", 0.05) : Qt.alpha(text, 0.07)
        if (bordered)
            return Qt.alpha(text, dark ? 0.08 : 0.07)
        return dark ? mix(base, text, 0.055) : mix(base, text, 0.14)
    }
    readonly property color keyPressed: glass ? Qt.alpha(text, dark ? 0.24 : 0.18) : mix(key, text, dark ? 0.2 : 0.13)
    readonly property color specialKeyPressed: glass ? Qt.alpha(text, dark ? 0.2 : 0.16) : mix(specialKey, text, dark ? 0.16 : 0.12)
    readonly property color keyBorder: bordered ? Qt.alpha(text, dark ? 0.22 : 0.26) : glass ? Qt.alpha("#ffffff", dark ? 0.12 : 0.7) : "transparent"
    readonly property color keyShadow: raised ? Qt.alpha("#000000", dark ? 0.55 : 0.2) : "transparent"
    readonly property color keySheen: glass || raised ? Qt.alpha("#ffffff", dark ? 0.045 : 0.0) : "transparent"
    readonly property color keyText: text
    readonly property color keyHint: Qt.alpha(text, dark ? 0.5 : 0.55)
    readonly property color bubble: dark ? mix(base, text, 0.3) : "#ffffff"
    readonly property color bubbleShadow: Qt.alpha("#000000", dark ? 0.5 : 0.22)
    readonly property color ripple: Qt.alpha(mix(accent, text, 0.35), dark ? 0.2 : 0.14)
    readonly property color stripText: text
    readonly property color divider: Qt.alpha(text, dark ? 0.12 : 0.14)

    readonly property real radius: Settings.keyRadius
    readonly property real gap: Settings.keyGap
    readonly property real labelScale: Settings.labelScale

    readonly property real plasmaFactor: Kirigami.Units.longDuration / 200
    readonly property bool animated: Settings.animationsEnabled && plasmaFactor > 0

    function duration(ms) {
        return animated ? Math.max(1, Math.round(ms * plasmaFactor / Settings.animationSpeed)) : 0
    }

    function mix(from, to, amount) {
        const a = Qt.color(from)
        const b = Qt.color(to)
        return Qt.rgba(a.r + (b.r - a.r) * amount, a.g + (b.g - a.g) * amount, a.b + (b.b - a.b) * amount, a.a + (b.a - a.a) * amount)
    }
}
