pragma ComponentBehavior: Bound

import QtQuick
import org.kde.kirigami as Kirigami
import org.devl0rd.kboard.config
import org.devl0rd.kboard.settings
import "VisibilityScenes.js" as VisibilityScenes
import "Ease.js" as Ease

Item {
    id: preview

    readonly property int mode: SystemStatus.virtualKeyboardMode < 0 ? 1 : SystemStatus.virtualKeyboardMode
    readonly property string input: VisibilityScenes.input(clock.step)
    readonly property bool opens: VisibilityScenes.opens(mode, input)
    readonly property string caption: VisibilityScenes.caption(clock.step, mode)
    readonly property real shown: {
        const prefix = input === "mouse" ? "mouse" : "touch";
        if (clock.step.startsWith("swipe")) {
            return opens ? 1 - clock.progress("swipe") : 0;
        }
        if (!opens) {
            return 0;
        }
        return clock.progress(prefix + "Show") * (1 - clock.progress(prefix + "Hide"));
    }
    readonly property point field: Qt.point(screen.width / 2, screen.height * 0.34)

    SceneClock {
        id: clock
        steps: VisibilityScenes.steps(Settings.hideOnSwipeDown)
        stillTime: 1600
    }

    ScreenFrame {
        id: screen
        anchors.fill: parent
        aspect: 16 / 9
        accent: Settings.accentMode === 1 ? Settings.accentColor : Kirigami.Theme.highlightColor
        window.focused: preview.shown > 0.5 || clock.step.endsWith("Hold")
        window.fieldGlow: clock.step.endsWith("Tap") ? Ease.pulse(clock.raw(clock.step)) : 0
        window.y: screenHeight * 0.06
        window.height: screenHeight * 0.45

        MiniKeyboard {
            id: keyboard
            width: parent.width
            height: parent.height * Settings.heightLandscape
            y: parent.height - height * preview.shown
            visible: preview.shown > 0.001
            showHints: false
            backdrop: screen.wallpaper
        }

        Finger {
            readonly property real approach: clock.step.endsWith("Move") ? clock.progress(clock.step) : 1
            readonly property bool swiping: clock.step.startsWith("swipe")
            mouse: preview.input === "mouse"
            size: Kirigami.Units.gridUnit * (mouse ? 1.1 : 1.3)
            x: swiping ? parent.width * 0.5 - width / 2 : Ease.lerp(parent.width * (mouse ? 0.85 : 0.2), parent.width * 0.3, approach) - width / 2
            y: swiping ? keyboard.y + keyboard.height * 0.3 + keyboard.height * clock.progress("swipe") - height / 2 : Ease.lerp(parent.height * 0.9, parent.height * 0.44, approach) - height / 2
            down: clock.step.endsWith("Tap") || clock.step === "swipe" || clock.step === "swipeTouch" ? 1 : 0
            opacity: clock.step === "swipeRest" || clock.step === "rest" ? 0 : 1
        }
    }
}
