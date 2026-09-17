pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.devl0rd.kboard.config
import org.devl0rd.kboard.settings
import "GlidePath.js" as GlidePath
import "GlideScenes.js" as GlideScenes
import "Ease.js" as Ease

Item {
    id: preview

    readonly property string scene: GlideScenes.scene(clock.step)
    readonly property string caption: GlideScenes.captions[scene]
    readonly property string word: "kboard"
    readonly property var wordPoints: {
        keyboard.width;
        keyboard.height;
        return GlidePath.smooth(word.split("").map(letter => keyboard.keyCenter(letter)), 8);
    }
    readonly property real glideT: clock.progress("glide")
    readonly property real cursorT: clock.progress("cursor")
    readonly property real deleteT: clock.progress("delete")
    readonly property point spaceCenter: keyboard.keyCenter("space")
    readonly property rect backspaceRect: keyboard.keyRect("backspace")

    SceneClock {
        id: clock
        steps: GlideScenes.steps({ glide: Settings.glideTyping, cursor: Settings.spaceCursorSwipe, backspace: Settings.backspaceSwipeDelete })
        stillTime: Settings.glideTyping ? 2000 : 0
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: Kirigami.Units.largeSpacing

        TextLine {
            id: line
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: keyboard.width
            readonly property string cursorText: "Move the cursor right here"
            readonly property string deleteText: "Delete these extra words"
            text: {
                switch (preview.scene) {
                case "glide":
                    return preview.glideT >= 1 ? "Try kboard " : "Try ";
                case "cursor":
                    return cursorText;
                case "delete":
                    return clock.step === "deleteHold" ? "Delete these " : deleteText;
                }
                return "";
            }
            caret: preview.scene === "cursor" ? Math.round(Ease.lerp(cursorText.length, 9, Ease.pulse(preview.cursorT * 0.999))) : text.length
            markStart: preview.scene === "glide" && preview.glideT >= 1 ? 4 : -1
            markLength: 6
            mark: 1 - clock.raw("glideHold")
            selectStart: preview.scene === "delete" && clock.step === "delete" ? deleteText.length - GlidePath.wordSelection(deleteText, 13, preview.deleteT) : -1
            selectLength: selectStart >= 0 ? deleteText.length - selectStart : 0
        }

        MiniKeyboard {
            id: keyboard
            Layout.alignment: Qt.AlignHCenter
            Layout.fillHeight: true
            Layout.preferredWidth: Math.min(parent.width, height * 2.9)
            showHints: false
            litKeys: preview.scene === "glide" && clock.step === "glide" ? preview.word.split("").filter((letter, index) => index / (preview.word.length - 1) <= preview.glideT + 0.02) : []
            trail: preview.scene === "glide" && Settings.glideTrail ? GlidePath.along(preview.wordPoints, preview.glideT) : []
            trailOpacity: clock.step === "glideHold" ? 1 - clock.raw("glideHold") : 1
            pressedKey: preview.scene === "cursor" ? "space" : (preview.scene === "delete" && clock.step !== "deleteHold" ? "backspace" : "")
            press: clock.step === "cursor" || clock.step === "delete" ? 1 : 0
        }
    }

    Finger {
        readonly property point spot: {
            switch (preview.scene) {
            case "glide":
                return preview.wordPoints.length > 0 ? GlidePath.along(preview.wordPoints, preview.glideT).slice(-1)[0] : Qt.point(0, 0);
            case "cursor":
                return Qt.point(preview.spaceCenter.x - keyboard.unitW * 1.5 * Ease.pulse(preview.cursorT), preview.spaceCenter.y);
            case "delete":
                return Qt.point(preview.backspaceRect.x + preview.backspaceRect.width / 2 - keyboard.unitW * 4 * preview.deleteT, preview.backspaceRect.y + preview.backspaceRect.height / 2);
            }
            return Qt.point(-100, -100);
        }
        visible: preview.scene !== "off" && !clock.step.endsWith("Hold")
        x: keyboard.x + spot.x - width / 2
        y: keyboard.y + spot.y - height / 2
        down: clock.step.endsWith("In") ? clock.raw(clock.step) : 1
    }
}
