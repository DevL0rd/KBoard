pragma Singleton

import QtQuick
import org.kde.kirigami as Kirigami
import org.devl0rd.kboard.config

QtObject {
    readonly property color accent: Settings.accentMode === 1 ? Settings.accentColor : Kirigami.Theme.highlightColor
    readonly property color ink: Kirigami.Theme.textColor
    readonly property color accentText: Kirigami.Theme.highlightedTextColor
    readonly property color danger: Kirigami.Theme.negativeTextColor
    readonly property bool animate: Settings.animationsEnabled
    readonly property real speed: Math.max(0.25, Settings.animationSpeed)

    function duration(ms) {
        return animate ? Math.round(ms / speed) : 0
    }
}
