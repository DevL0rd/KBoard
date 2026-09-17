pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.devl0rd.kboard.config
import org.devl0rd.kboard.settings

RowLayout {
    id: preview

    readonly property var accents: ["è", "é", "ê", "ë", "ē"]
    readonly property real holdT: longPress.raw("hold")
    readonly property real slideT: longPress.progress("slide")

    spacing: Kirigami.Units.gridUnit

    component Panel: ColumnLayout {
        id: panel
        property string title
        default property alias body: holder.data
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.preferredWidth: 1
        spacing: Kirigami.Units.smallSpacing

        Item {
            id: holder
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        QQC2.Label {
            text: panel.title
            font.weight: Font.DemiBold
            horizontalAlignment: Text.AlignHCenter
            Layout.fillWidth: true
        }
    }

    TypingLoop {
        id: tapping
        keys: ["w", "e", "r"]
        keyMs: 700
        restMs: 400
    }

    SceneClock {
        id: longPress
        steps: [
            { id: "pause", ms: 500 },
            { id: "hold", ms: Settings.longPressDelay * Motion.speed },
            { id: "open", ms: 300 },
            { id: "slide", ms: 900, ease: "inOutCubic" },
            { id: "release", ms: 900 }
        ]
    }

    Panel {
        title: "Tap"

        CloseUp {
            anchors.fill: parent
            centerKey: "e"
            rowsVisible: 3
            topFraction: 0.42
            keyboard.showHints: false
            keyboard.pressedKey: tapping.pressedKey
            keyboard.press: tapping.press
            keyboard.popupKey: Settings.keyPopup ? tapping.pressedKey : ""
            keyboard.popup: tapping.popup
            keyboard.rippleKey: Settings.ripple ? tapping.pressedKey : ""
            keyboard.ripple: tapping.ripple
            keyboard.rippleOrigin: Qt.point(0.35, 0.6)
        }
    }

    Panel {
        title: "Long press · " + Settings.longPressDelay + " ms"

        CloseUp {
            anchors.fill: parent
            centerKey: "e"
            rowsVisible: 3
            topFraction: 0.5
            keyboard.pressedKey: longPress.step === "pause" || longPress.step === "release" ? "" : "e"
            keyboard.press: 1
            keyboard.holdKey: "e"
            keyboard.hold: longPress.step === "hold" ? preview.holdT : 0
            keyboard.accentKey: "e"
            keyboard.accentOpen: longPress.step === "open" ? longPress.raw("open") : (longPress.step === "slide" ? 1 : (longPress.step === "release" ? 1 - longPress.raw("release") : 0))
            keyboard.accentIndex: longPress.step === "open" ? 0 : Math.round(preview.slideT * 2)
        }
    }

    Panel {
        title: "Key repeat"

        RepeatDemo {
            anchors.fill: parent
            anchors.margins: Kirigami.Units.smallSpacing
        }
    }
}
