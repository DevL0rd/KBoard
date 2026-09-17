pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Shapes
import org.kde.kirigami as Kirigami

Item {
    id: wave

    property real amplitude: 1
    property real frequency: 9
    property real decay: 5
    property real progress: 1
    property real energy: 1
    property int samples: 96
    property color color: Kirigami.Theme.highlightColor
    property real lineWidth: 2

    readonly property var points: {
        const list = [];
        const mid = height / 2;
        for (let i = 0; i <= samples; ++i) {
            const t = i / samples;
            const envelope = Math.exp(-decay * t) * Math.min(1, t * 40);
            const visible = t <= progress ? 1 : 0;
            list.push(Qt.point(t * width, mid - Math.sin(t * frequency * Math.PI * 2) * envelope * amplitude * energy * visible * mid * 0.92));
        }
        return list;
    }

    Rectangle {
        anchors.verticalCenter: parent.verticalCenter
        width: parent.width
        height: 1
        color: Qt.alpha(Kirigami.Theme.textColor, 0.15)
    }

    Shape {
        anchors.fill: parent
        preferredRendererType: Shape.CurveRenderer

        ShapePath {
            strokeWidth: wave.lineWidth
            strokeColor: wave.color
            fillColor: "transparent"
            joinStyle: ShapePath.RoundJoin
            capStyle: ShapePath.RoundCap
            PathPolyline {
                path: wave.points
            }
        }
    }
}
