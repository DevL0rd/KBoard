pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.devl0rd.kboard.config
import org.devl0rd.kboard.settings
import "TypingScript.js" as TypingScript
import "Ease.js" as Ease

RowLayout {
    id: preview

    readonly property var frames: TypingScript.build({
        suggestions: Settings.suggestions,
        autocorrect: Settings.autocorrect,
        nextWordPrediction: Settings.nextWordPrediction,
        autoCapitalize: Settings.autoCapitalize,
        doubleSpacePeriod: Settings.doubleSpacePeriod,
        emojiSuggestions: Settings.emojiSuggestions,
        expansions: Settings.textExpansions
    })
    readonly property int index: Math.max(0, Number(clock.step.substring(1)))
    readonly property var frame: frames[Math.min(index, frames.length - 1)]
    readonly property real local: clock.raw(clock.step)
    property string caption: ""

    spacing: Kirigami.Units.gridUnit * 1.5

    onFrameChanged: {
        if (frame.caption.length > 0) {
            caption = frame.caption;
        }
    }

    SceneClock {
        id: clock
        steps: preview.frames.map((frame, index) => ({ id: "f" + index, ms: frame.ms }))
        stillTime: preview.frames.slice(0, preview.frames.length - 1).reduce((sum, frame) => sum + frame.ms, 0) - 10
    }

    ColumnLayout {
        Layout.fillWidth: true
        Layout.preferredWidth: 1
        Layout.alignment: Qt.AlignVCenter
        spacing: Kirigami.Units.largeSpacing

        TextLine {
            Layout.fillWidth: true
            text: preview.frame.text
            markStart: preview.frame.markStart
            markLength: preview.frame.markLength
            mark: preview.frame.markStart >= 0 ? 1 - Ease.segment(preview.local, 0.7, 1) : 0
            fontSize: Kirigami.Units.gridUnit * 1.05
        }

        SuggestionBar {
            Layout.fillWidth: true
            visible: Settings.suggestions
            suggestions: preview.frame.suggestions
        }
    }

    MiniKeyboard {
        Layout.preferredWidth: Math.min(parent.width * 0.42, (parent.height - Kirigami.Units.gridUnit * 2) * 2.4)
        Layout.preferredHeight: Layout.preferredWidth / 2.4
        Layout.alignment: Qt.AlignBottom
        showHints: false
        shifted: preview.frame.shifted
        pressedKey: preview.frame.key
        press: Ease.pulse(Ease.segment(preview.local, 0, 0.6))
        rippleKey: Settings.ripple ? preview.frame.key : ""
        ripple: Ease.segment(preview.local, 0, 0.8)
        popupKey: Settings.keyPopup ? preview.frame.key : ""
        popup: Ease.pulse(Ease.segment(preview.local, 0, 0.9))
    }
}
