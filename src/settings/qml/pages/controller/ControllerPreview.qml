pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.devl0rd.kboard.config
import org.devl0rd.kboard.settings
import "Glyphs.js" as Glyphs
import "ControllerActions.js" as ControllerActions

RowLayout {
    id: preview

    readonly property var pad: Modules.gamepad ? Modules.gamepad.api : null
    readonly property string type: Glyphs.typeOf(pad)
    property string pressed: ""
    readonly property string touring: ControllerActions.tour[clock.cycle % ControllerActions.tour.length]
    readonly property string shown: idle.running ? pressed : (clock.step === "hold" || clock.step === "move" ? touring : "")
    readonly property var lit: pad && pad.heldButtons.length > 0 ? pad.heldButtons : (shown.length > 0 ? [shown] : [])
    readonly property string caption: pad ? (pad.connected ? pad.name + " connected · press any button" : "No controller connected · showing what each button does") : "Controller support isn't available"

    spacing: Kirigami.Units.gridUnit * 2

    SceneClock {
        id: clock
        active: !idle.running
        steps: [{ id: "pause", ms: 300 }, { id: "move", ms: 250 }, { id: "hold", ms: 1300 }]
    }

    Timer {
        id: idle
        interval: 4000
    }

    Connections {
        target: preview.pad
        ignoreUnknownSignals: true
        function onButtonPressed(button) {
            preview.pressed = button;
            idle.restart();
        }
    }

    GamepadDiagram {
        Layout.fillHeight: true
        Layout.preferredWidth: height * 200 / 132
        lit: preview.lit
        controllerType: preview.type
        glyphs: ({ a: Glyphs.label(preview.pad, "a"), b: Glyphs.label(preview.pad, "b"), x: Glyphs.label(preview.pad, "x"), y: Glyphs.label(preview.pad, "y") })
        leftStick: preview.pad ? preview.pad.leftStick : Qt.point(0, 0)
        rightStick: preview.pad ? preview.pad.rightStick : Qt.point(0, 0)
        accent: Settings.accentMode === 1 ? Settings.accentColor : Kirigami.Theme.highlightColor
        opacity: Settings.controllerEnabled ? 1 : 0.4
    }

    ColumnLayout {
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignVCenter
        spacing: Kirigami.Units.largeSpacing

        RowLayout {
            spacing: Kirigami.Units.largeSpacing
            opacity: preview.shown.length > 0 ? 1 : 0.35

            ButtonGlyph {
                text: preview.shown.length > 0 ? Glyphs.label(preview.pad, preview.shown) : "·"
                active: preview.shown.length > 0
            }

            Kirigami.Heading {
                level: 3
                text: preview.shown.length > 0 ? ControllerActions.actionFor(preview.shown) : "Press a button"
                Layout.fillWidth: true
                elide: Text.ElideRight
            }
        }

        RowLayout {
            spacing: Kirigami.Units.smallSpacing

            QQC2.Label {
                text: "Open the keyboard"
                opacity: 0.75
            }

            Repeater {
                model: Settings.controllerOpenChord.split("+").filter(Boolean)

                ButtonGlyph {
                    required property string modelData
                    text: Glyphs.label(preview.pad, modelData)
                }
            }
        }
    }
}
