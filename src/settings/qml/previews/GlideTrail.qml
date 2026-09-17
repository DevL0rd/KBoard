pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Shapes

Item {
    id: trailLayer
    property var kb
    anchors.fill: parent

    Shape {
        id: trailShape
        anchors.fill: parent
        visible: trailLayer.kb.trail.length > 1 && trailLayer.kb.trailOpacity > 0
        opacity: trailLayer.kb.trailOpacity
        z: 5
        preferredRendererType: Shape.CurveRenderer

        ShapePath {
            strokeWidth: Math.max(2, 7 * trailLayer.kb.metric)
            strokeColor: Qt.alpha(trailLayer.kb.accent, 0.35)
            fillColor: "transparent"
            capStyle: ShapePath.RoundCap
            joinStyle: ShapePath.RoundJoin
            PathPolyline {
                path: trailLayer.kb.trail
            }
        }

        ShapePath {
            strokeWidth: Math.max(1.2, 3 * trailLayer.kb.metric)
            strokeColor: trailLayer.kb.accent
            fillColor: "transparent"
            capStyle: ShapePath.RoundCap
            joinStyle: ShapePath.RoundJoin
            PathPolyline {
                path: trailLayer.kb.trail
            }
        }
    }

    Rectangle {
        readonly property point head: trailLayer.kb.trail.length > 0 ? trailLayer.kb.trail[trailLayer.kb.trail.length - 1] : Qt.point(0, 0)
        visible: trailShape.visible
        z: 6
        width: Math.max(6, 16 * trailLayer.kb.metric)
        height: width
        radius: width / 2
        x: head.x - width / 2
        y: head.y - height / 2
        color: Qt.alpha(trailLayer.kb.accent, 0.35)
        border.width: Math.max(1, 2 * trailLayer.kb.metric)
        border.color: trailLayer.kb.accent
        opacity: trailLayer.kb.trailOpacity
    }
}
