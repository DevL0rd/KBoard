pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Shapes
import org.kde.kirigami as Kirigami

Item {
    id: orb

    property string mode: "idle"
    property real level: 0
    property real phase: 0
    property real spin: 0
    property real progress: -1
    signal clicked()

    readonly property bool live: mode === "listening"
    readonly property bool showsRing: mode === "busy" || mode === "download"
    readonly property color coreColor: mode === "error" ? VoiceStyle.danger : VoiceStyle.accent

    Accessible.role: Accessible.Button
    Accessible.name: i18nc("@action:button", "Microphone")
    Accessible.onPressAction: clicked()

    Shape {
        anchors.centerIn: parent
        width: orb.width * 3
        height: width
        preferredRendererType: Shape.CurveRenderer
        opacity: orb.live ? 1 : 0.5

        Behavior on opacity { NumberAnimation { duration: VoiceStyle.duration(300); easing.type: Easing.OutCubic } }

        ShapePath {
            strokeWidth: 0
            strokeColor: "transparent"
            fillGradient: RadialGradient {
                centerX: orb.width * 1.5
                centerY: orb.width * 1.5
                centerRadius: orb.width * (1.25 + orb.level * 0.35)
                focalX: centerX
                focalY: centerY
                GradientStop { position: 0; color: Qt.alpha(orb.coreColor, 0.22 + orb.level * 0.14) }
                GradientStop { position: 0.45; color: Qt.alpha(orb.coreColor, 0.07) }
                GradientStop { position: 1; color: Qt.alpha(orb.coreColor, 0) }
            }
            PathAngleArc {
                centerX: orb.width * 1.5
                centerY: orb.width * 1.5
                radiusX: orb.width * 1.5
                radiusY: orb.width * 1.5
                sweepAngle: 360
            }
        }
    }

    Repeater {
        model: 3

        Rectangle {
            required property int index
            readonly property real local: (orb.phase + index / 3) % 1
            readonly property real eased: 1 - Math.pow(1 - local, 2.2)

            anchors.centerIn: parent
            width: orb.width
            height: width
            radius: width / 2
            color: "transparent"
            border.width: Math.max(1.5, orb.width * 0.018)
            border.color: VoiceStyle.accent
            visible: VoiceStyle.animate && orb.live
            scale: 1 + eased * (0.55 + orb.level * 0.9)
            opacity: (1 - local) * (0.18 + orb.level * 0.55)
        }
    }

    Repeater {
        model: [
            { size: 1.32, grow: 0.42, fill: 0.10, border: 0.22 },
            { size: 1.14, grow: 0.22, fill: 0.16, border: 0 }
        ]

        Rectangle {
            required property var modelData

            anchors.centerIn: parent
            width: orb.width * (modelData.size + orb.level * modelData.grow)
            height: width
            radius: width / 2
            color: Qt.alpha(orb.coreColor, modelData.fill + orb.level * modelData.fill)
            border.width: modelData.border > 0 ? 1 : 0
            border.color: Qt.alpha(orb.coreColor, modelData.border)
            opacity: orb.live ? 1 : 0.4

            Behavior on opacity { NumberAnimation { duration: VoiceStyle.duration(250); easing.type: Easing.OutCubic } }
        }
    }

    Shape {
        id: ring

        readonly property real thickness: Math.max(3, orb.width * 0.045)
        readonly property real radius: (width - thickness) / 2
        readonly property bool determinate: orb.progress >= 0

        anchors.centerIn: parent
        width: orb.width * 1.24
        height: width
        preferredRendererType: Shape.CurveRenderer
        visible: orb.showsRing

        ShapePath {
            strokeColor: Qt.alpha(VoiceStyle.ink, 0.12)
            strokeWidth: ring.thickness
            fillColor: "transparent"
            PathAngleArc {
                centerX: ring.width / 2
                centerY: ring.height / 2
                radiusX: ring.radius
                radiusY: ring.radius
                sweepAngle: 360
            }
        }

        ShapePath {
            strokeColor: VoiceStyle.accent
            strokeWidth: ring.thickness
            fillColor: "transparent"
            capStyle: ShapePath.RoundCap
            PathAngleArc {
                centerX: ring.width / 2
                centerY: ring.height / 2
                radiusX: ring.radius
                radiusY: ring.radius
                startAngle: ring.determinate ? -90 : orb.spin - 90
                sweepAngle: ring.determinate ? Math.max(1, 360 * Math.min(1, orb.progress)) : 90
            }
        }
    }

    Rectangle {
        id: core

        anchors.centerIn: parent
        width: orb.width
        height: width
        radius: width / 2
        scale: (area.pressed ? 0.94 : 1) * (1 + orb.level * 0.08)
        opacity: orb.mode === "paused" || orb.mode === "download" ? 0.82 : 1
        gradient: Gradient {
            GradientStop { position: 0; color: Qt.lighter(orb.coreColor, 1.18) }
            GradientStop { position: 1; color: Qt.darker(orb.coreColor, 1.22) }
        }

        Behavior on scale {
            enabled: VoiceStyle.animate
            SpringAnimation { spring: 5; damping: 0.35; epsilon: 0.001 }
        }

        Rectangle {
            anchors.fill: parent
            anchors.margins: 1
            radius: width / 2
            color: "transparent"
            border.width: 1
            border.color: Qt.alpha(VoiceStyle.accentText, 0.28)
        }

        Kirigami.Icon {
            anchors.centerIn: parent
            width: Math.round(parent.width * 0.42)
            height: width
            isMask: true
            color: VoiceStyle.accentText
            source: ({
                download: "go-down-symbolic",
                error: "view-refresh-symbolic",
                paused: "media-playback-start-symbolic"
            })[orb.mode] ?? "audio-input-microphone-symbolic"
        }

        MouseArea {
            id: area
            anchors.fill: parent
            anchors.margins: -Kirigami.Units.largeSpacing
            onClicked: orb.clicked()
        }
    }
}
