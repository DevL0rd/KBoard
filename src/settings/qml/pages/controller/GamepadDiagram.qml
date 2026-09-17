pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Shapes
import org.kde.kirigami as Kirigami

Item {
    id: diagram

    property var lit: []
    property string controllerType: "xbox"
    property var glyphs: ({})
    property point leftStick: Qt.point(0, 0)
    property point rightStick: Qt.point(0, 0)
    property color accent: Kirigami.Theme.highlightColor
    readonly property real unit: Math.min(width / 200, height / 132)
    readonly property bool symmetric: controllerType === "playstation"

    component Part: GamepadPart {
        property real cx
        property real cy
        property real w
        property real h
        lit: diagram.lit
        accent: diagram.accent
        glyph: diagram.glyphs[button] || ""
        x: canvas.x + (cx - w / 2) * diagram.unit
        y: canvas.y + (cy - h / 2) * diagram.unit
        width: w * diagram.unit
        height: h * diagram.unit
    }

    Item {
        id: canvas
        width: 200 * diagram.unit
        height: 132 * diagram.unit
        anchors.centerIn: parent

        Shape {
            width: 200
            height: 132
            scale: diagram.unit
            transformOrigin: Item.TopLeft
            preferredRendererType: Shape.CurveRenderer

            ShapePath {
                strokeWidth: 1.2
                strokeColor: Qt.alpha(Kirigami.Theme.textColor, 0.35)
                fillGradient: LinearGradient {
                    x1: 0
                    y1: 20
                    x2: 0
                    y2: 128
                    GradientStop {
                        position: 0
                        color: Qt.lighter(Kirigami.Theme.backgroundColor, 1.35)
                    }
                    GradientStop {
                        position: 1
                        color: Kirigami.Theme.backgroundColor
                    }
                }
                PathSvg {
                    path: "M 52 22 C 30 22 20 32 14 56 L 5 100 C 1 120 14 131 28 126 C 40 122 46 106 58 98 L 142 98 C 154 106 160 122 172 126 C 186 131 199 120 195 100 L 186 56 C 180 32 170 22 148 22 Z"
                }
            }
        }
    }

    Part { button: "lefttrigger"; cx: 58; cy: 7; w: 28; h: 9; radius: 4 * diagram.unit }
    Part { button: "righttrigger"; cx: 142; cy: 7; w: 28; h: 9; radius: 4 * diagram.unit }
    Part { button: "leftshoulder"; cx: 54; cy: 17; w: 38; h: 7; radius: 3 * diagram.unit }
    Part { button: "rightshoulder"; cx: 146; cy: 17; w: 38; h: 7; radius: 3 * diagram.unit }

    Part {
        button: "leftstick"
        cx: diagram.symmetric ? 76 : 50
        cy: diagram.symmetric ? 78 : 50
        w: 24
        h: 24
        radius: width / 2
        transform: Translate {
            x: diagram.leftStick.x * 4 * diagram.unit
            y: diagram.leftStick.y * 4 * diagram.unit
        }
    }

    Part {
        button: "rightstick"
        cx: diagram.symmetric ? 124 : 122
        cy: 78
        w: 24
        h: 24
        radius: width / 2
        transform: Translate {
            x: diagram.rightStick.x * 4 * diagram.unit
            y: diagram.rightStick.y * 4 * diagram.unit
        }
    }

    Repeater {
        model: [
            { button: "dpup", dx: 0, dy: -1 }, { button: "dpdown", dx: 0, dy: 1 },
            { button: "dpleft", dx: -1, dy: 0 }, { button: "dpright", dx: 1, dy: 0 }
        ]

        Part {
            required property var modelData
            button: modelData.button
            cx: (diagram.symmetric ? 50 : 78) + modelData.dx * 8.5
            cy: (diagram.symmetric ? 50 : 78) + modelData.dy * 8.5
            w: 9
            h: 9
            radius: 2 * diagram.unit
        }
    }

    Repeater {
        model: [
            { button: "y", dx: 0, dy: -1 }, { button: "a", dx: 0, dy: 1 },
            { button: "x", dx: -1, dy: 0 }, { button: "b", dx: 1, dy: 0 }
        ]

        Part {
            required property var modelData
            button: modelData.button
            cx: 150 + modelData.dx * 12
            cy: 50 + modelData.dy * 12
            w: 12
            h: 12
            radius: width / 2
        }
    }

    Part { button: "back"; cx: 86; cy: 48; w: 11; h: 6; radius: height / 2 }
    Part { button: "start"; cx: 114; cy: 48; w: 11; h: 6; radius: height / 2 }
    Part { button: "guide"; cx: 100; cy: 34; w: 12; h: 12; radius: width / 2 }
}
