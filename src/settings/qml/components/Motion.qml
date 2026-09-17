pragma Singleton
pragma ComponentBehavior: Bound

import QtQuick
import org.devl0rd.kboard.config

QtObject {
    readonly property bool enabled: Settings.animationsEnabled
    readonly property real speed: Math.max(0.25, Settings.animationSpeed)

    function ms(base) {
        return enabled ? Math.round(base / speed) : 0;
    }

    readonly property int quick: ms(120)
    readonly property int short: ms(180)
    readonly property int medium: ms(260)
    readonly property int long: ms(420)
}
