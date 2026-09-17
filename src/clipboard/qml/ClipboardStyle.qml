pragma Singleton

import QtQuick
import org.kde.kirigami as Kirigami
import org.devl0rd.kboard.config

QtObject {
    readonly property color accentColor: Settings.accentMode === 1 ? Settings.accentColor : Kirigami.Theme.highlightColor
    readonly property real cardRadius: Kirigami.Units.cornerRadius * 2.5

    function duration(ms) {
        return Settings.animationsEnabled ? Math.round(ms / Settings.animationSpeed) : 0
    }
}
