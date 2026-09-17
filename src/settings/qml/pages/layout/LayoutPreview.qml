pragma ComponentBehavior: Bound

import QtQuick
import org.kde.kirigami as Kirigami
import org.devl0rd.kboard.config
import org.devl0rd.kboard.settings
import "SplitRule.js" as SplitRule
import "Ease.js" as Ease

Item {
    id: preview

    readonly property var screens: [
        { name: "Portrait", width: 1080, height: 1920 },
        { name: "Landscape", width: 1920, height: 1080 },
        { name: "Ultrawide", width: 3440, height: 1440 }
    ]
    readonly property int rows: 4 + (Settings.showNumberRow ? 1 : 0) + (Settings.showDesktopRow ? 1 : 0)
    readonly property int from: clock.step.startsWith("to") ? (Number(clock.step.substring(2)) + 2) % 3 : Number(clock.step.substring(4))
    readonly property int to: clock.step.startsWith("to") ? Number(clock.step.substring(2)) : from
    readonly property real t: clock.step.startsWith("to") ? clock.progress(clock.step) : 0
    readonly property int layoutIndex: Settings.layouts.length > 0 ? clock.cycle % Settings.layouts.length : 0
    readonly property string layoutName: {
        const id = Settings.layouts[layoutIndex] || Settings.activeLayout;
        const info = Modules.layouts ? Modules.layouts.api.layout(id) : {};
        return info.shortName || info.name || id;
    }
    readonly property var shown: screens[t < 0.5 ? from : to]
    readonly property string caption: shown.name + " " + shown.width + "×" + shown.height + " · " + (splitFor(t < 0.5 ? from : to) > 0.5 ? "splits into two thumb halves" : "full width")

    function share(index) {
        return screens[index].width < screens[index].height ? Settings.heightPortrait : Settings.heightLandscape;
    }

    function splitFor(index) {
        const s = screens[index];
        return SplitRule.shouldSplit(Settings.splitMode, s.width, s.height, share(index), rows) ? 1 : 0;
    }

    SceneClock {
        id: clock
        steps: [
            { id: "hold0", ms: 1300 }, { id: "to1", ms: 1400, ease: "inOutCubic" },
            { id: "hold1", ms: 1300 }, { id: "to2", ms: 1400, ease: "inOutCubic" },
            { id: "hold2", ms: 1300 }, { id: "to0", ms: 1400, ease: "inOutCubic" }
        ]
        stillTime: 2700 + 650
    }

    ScreenFrame {
        id: screen
        anchors.fill: parent
        aspect: Ease.lerp(preview.screens[preview.from].width / preview.screens[preview.from].height, preview.screens[preview.to].width / preview.screens[preview.to].height, preview.t)
        accent: Settings.accentMode === 1 ? Settings.accentColor : Kirigami.Theme.highlightColor
        window.focused: true
        window.text: "Hallo"
        window.height: screenHeight * (1 - keyboard.height / screenHeight) - screenHeight * 0.12

        MiniKeyboard {
            id: keyboard
            anchors.bottom: parent.bottom
            width: parent.width
            height: parent.height * Ease.lerp(preview.share(preview.from), preview.share(preview.to), preview.t)
            metric: Math.max(0.3, Math.min(parent.width, parent.height * 1.6) / 560)
            backdrop: screen.wallpaper
            showHints: false
            split: Ease.lerp(preview.splitFor(preview.from), preview.splitFor(preview.to), preview.t)
            spaceLabel: preview.layoutName
        }
    }
}
