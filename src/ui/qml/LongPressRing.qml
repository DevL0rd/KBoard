pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Shapes

Item {
    id: ring

    property bool running
    property int duration: 300
    property real progress: 0

    visible: progress > 0.2
    opacity: Math.min(1, (progress - 0.2) * 4)

    Shape {
        anchors.fill: parent
        preferredRendererType: Shape.CurveRenderer

        ShapePath {
            fillColor: "transparent"
            strokeColor: Theme.accent
            strokeWidth: Math.max(1.5, ring.width * 0.08)
            capStyle: ShapePath.RoundCap

            PathAngleArc {
                centerX: ring.width / 2
                centerY: ring.height / 2
                radiusX: ring.width / 2 - 1
                radiusY: ring.height / 2 - 1
                startAngle: -90
                sweepAngle: 360 * ring.progress
            }
        }
    }

    NumberAnimation {
        id: fill

        target: ring
        property: "progress"
        from: 0
        to: 1
        duration: ring.duration
    }

    onRunningChanged: {
        if (running) {
            fill.restart()
        } else {
            fill.stop()
            progress = 0
        }
    }
}
