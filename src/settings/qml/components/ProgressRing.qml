pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Shapes
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

Item {
    id: ring

    property real value: 0
    property real thickness: Math.max(3, width * 0.1)
    property color color: Kirigami.Theme.highlightColor

    implicitWidth: Kirigami.Units.gridUnit * 2.2
    implicitHeight: implicitWidth

    Shape {
        anchors.fill: parent
        preferredRendererType: Shape.CurveRenderer

        ShapePath {
            strokeColor: Qt.alpha(Kirigami.Theme.textColor, 0.12)
            strokeWidth: ring.thickness
            fillColor: "transparent"
            PathAngleArc {
                centerX: ring.width / 2
                centerY: ring.height / 2
                radiusX: (ring.width - ring.thickness) / 2
                radiusY: radiusX
                startAngle: 0
                sweepAngle: 360
            }
        }

        ShapePath {
            strokeColor: ring.color
            strokeWidth: ring.thickness
            fillColor: "transparent"
            capStyle: ShapePath.RoundCap
            PathAngleArc {
                centerX: ring.width / 2
                centerY: ring.height / 2
                radiusX: (ring.width - ring.thickness) / 2
                radiusY: radiusX
                startAngle: -90
                sweepAngle: 360 * Math.max(0.005, Math.min(1, ring.value))
            }
        }
    }

    QQC2.Label {
        anchors.centerIn: parent
        text: Math.round(ring.value * 100)
        font.pixelSize: ring.width * 0.3
        font.features: { "tnum": 1 }
    }
}
