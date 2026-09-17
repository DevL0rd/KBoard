pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Shapes

Item {
    id: icon

    property string name
    property color color: Theme.keyText
    property string shiftState: "off"
    property real strokeWidth: 1.9

    readonly property var outlines: ({
        shift: "M12 4.2 L20.2 12.9 H15.6 V19.2 H8.4 V12.9 H3.8 Z",
        backspace: "M8.6 5.5 H18.8 A2.2 2.2 0 0 1 21 7.7 V16.3 A2.2 2.2 0 0 1 18.8 18.5 H8.6 L2.8 12 Z M11.6 9.3 L16.6 14.7 M16.6 9.3 L11.6 14.7",
        enter: "M19.5 5.5 V10.5 A2.5 2.5 0 0 1 17 13 H5 M9 9 L5 13 L9 17",
        globe: "M12 3.5 A8.5 8.5 0 1 0 12 20.5 A8.5 8.5 0 1 0 12 3.5 Z M12 3.5 C8.6 7.4 8.6 16.6 12 20.5 C15.4 16.6 15.4 7.4 12 3.5 M3.8 9.4 H20.2 M3.8 14.6 H20.2",
        emoji: "M12 3.5 A8.5 8.5 0 1 0 12 20.5 A8.5 8.5 0 1 0 12 3.5 Z M8 14 C9.2 16.3 14.8 16.3 16 14",
        left: "M14.5 6 L8.5 12 L14.5 18",
        right: "M9.5 6 L15.5 12 L9.5 18",
        up: "M6 14.5 L12 8.5 L18 14.5",
        down: "M6 9.5 L12 15.5 L18 9.5",
        tab: "M3.5 12 H17 M12.5 7.5 L17 12 L12.5 16.5 M20.5 6.5 V17.5",
        "super": "M5 5 H11 V11 H5 Z M13 5 H19 V11 H13 Z M5 13 H11 V19 H5 Z M13 13 H19 V19 H13 Z",
        hide: "M4 5 H20 V13 H4 Z M7 8 H8 M10.5 8 H11.5 M14 8 H15 M8 10.5 H16 M9 16 L12 19 L15 16"
    })
    readonly property bool filled: name === "shift" && shiftState !== "off"
    readonly property real unit: Math.min(width, height) / 24

    implicitWidth: 24
    implicitHeight: 24

    Shape {
        id: shape

        anchors.centerIn: parent
        width: 24
        height: 24
        scale: icon.unit
        preferredRendererType: Shape.CurveRenderer

        ShapePath {
            strokeColor: icon.color
            strokeWidth: icon.strokeWidth
            fillColor: icon.filled ? icon.color : "transparent"
            capStyle: ShapePath.RoundCap
            joinStyle: ShapePath.RoundJoin

            Behavior on fillColor {
                ColorAnimation {
                    duration: Theme.duration(140)
                    easing.type: Easing.OutCubic
                }
            }

            PathSvg {
                path: icon.outlines[icon.name] ?? ""
            }
        }

        ShapePath {
            strokeColor: "transparent"
            fillColor: icon.name === "emoji" ? icon.color : "transparent"

            PathSvg {
                path: "M9.2 9.6 A1.1 1.1 0 1 0 9.2 9.61 Z M14.8 9.6 A1.1 1.1 0 1 0 14.8 9.61 Z"
            }
        }
    }

    Rectangle {
        anchors.horizontalCenter: shape.horizontalCenter
        y: icon.height / 2 + 9.2 * icon.unit
        width: 12 * icon.unit
        height: Math.max(1.5, 1.9 * icon.unit)
        radius: height / 2
        color: icon.color
        visible: icon.name === "shift"
        transformOrigin: Item.Center
        scale: icon.shiftState === "locked" ? 1 : 0
        opacity: scale

        Behavior on scale {
            NumberAnimation {
                duration: Theme.duration(180)
                easing.type: Easing.OutBack
            }
        }
    }
}
