pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Shapes
import org.kde.kirigami as Kirigami
import "KeyboardModel.js" as KeyboardModel
import "KeyStyle.js" as KeyStyle
import "Ease.js" as Ease

Repeater {
    id: keys
    property var kb
    model: keys.kb.model.keys

    Item {
        id: key

        required property var modelData
        readonly property var kb: keys.kb
        readonly property var geometry: KeyboardModel.splitX(modelData.x, modelData.w, key.kb.split, key.kb.splitHalfWidth)
        readonly property bool pressed: key.kb.pressedKey === modelData.id || (modelData.part !== "" && key.kb.pressedKey === "space")
        readonly property real pressAmount: pressed ? key.kb.press : 0
        readonly property bool lit: key.kb.litKeys.indexOf(modelData.id) >= 0
        readonly property real partOpacity: modelData.part === "whole" ? (key.kb.split > 0.001 ? 0 : 1) : (modelData.part === "" ? 1 : (key.kb.split > 0.001 ? 1 : 0))
        readonly property color face: KeyStyle.face(key.kb.keyStyle, modelData.kind, key.kb.keyPalette)

        x: geometry.x * key.kb.unitW + key.kb.gapPx / 2
        y: modelData.y * key.kb.unitH + key.kb.gapPx / 2
        width: Math.max(1, geometry.w * key.kb.unitW - key.kb.gapPx)
        height: Math.max(1, modelData.h * key.kb.unitH - key.kb.gapPx)
        opacity: partOpacity
        visible: opacity > 0
        scale: 1 - 0.07 * pressAmount
        z: pressed ? 2 : 1

        Rectangle {
            visible: key.kb.keyStyle === KeyStyle.Raised
            anchors.fill: parent
            anchors.topMargin: Math.max(1, 1.6 * key.kb.metric)
            anchors.bottomMargin: -Math.max(1, 1.6 * key.kb.metric)
            radius: face.radius
            color: KeyStyle.shadowColor(key.kb.keyPalette)
        }

        Rectangle {
            id: face
            anchors.fill: parent
            radius: Math.min(key.kb.radiusPx, height / 2)
            color: key.lit ? Ease.mix(key.face, key.kb.accent, 0.35) : Ease.mix(key.face, key.kb.accent, 0.3 * key.pressAmount)
            border.width: KeyStyle.borderWidth(key.kb.keyStyle)
            border.color: KeyStyle.borderColor(key.kb.keyStyle, key.kb.keyPalette)
            clip: true

            Rectangle {
                visible: key.kb.keyStyle === KeyStyle.Glass
                width: parent.width
                height: parent.height / 2
                radius: parent.radius
                gradient: Gradient {
                    GradientStop { position: 0; color: Qt.alpha("white", key.kb.darkTheme ? 0.1 : 0.35) }
                    GradientStop { position: 1; color: Qt.alpha("white", 0) }
                }
            }

            Rectangle {
                readonly property real size: Math.max(parent.width, parent.height) * 2.4 * Ease.outCubic(key.kb.ripple)
                visible: key.kb.rippleKey === key.modelData.id && key.kb.ripple > 0 && key.kb.ripple < 1
                x: parent.width * key.kb.rippleOrigin.x - size / 2
                y: parent.height * key.kb.rippleOrigin.y - size / 2
                width: size
                height: size
                radius: size / 2
                color: Qt.alpha(key.kb.accent, 0.45 * (1 - key.kb.ripple))
            }

            Shape {
                anchors.fill: parent
                visible: key.kb.holdKey === key.modelData.id && key.kb.hold > 0
                preferredRendererType: Shape.CurveRenderer

                ShapePath {
                    strokeWidth: 0
                    strokeColor: "transparent"
                    fillColor: Qt.alpha(key.kb.accent, 0.42)
                    startX: face.width / 2
                    startY: face.height / 2
                    PathAngleArc {
                        centerX: face.width / 2
                        centerY: face.height / 2
                        radiusX: Math.hypot(face.width, face.height) / 2
                        radiusY: radiusX
                        startAngle: -90
                        sweepAngle: 360 * key.kb.hold
                        moveToStart: false
                    }
                    PathLine {
                        x: face.width / 2
                        y: face.height / 2
                    }
                }
            }
        }

        Text {
            anchors.centerIn: parent
            anchors.verticalCenterOffset: key.modelData.kind === "space" ? 0 : height * 0.04
            visible: (key.modelData.label || "").length > 0
            text: key.modelData.kind === "char" && key.kb.shifted ? key.modelData.label.toUpperCase() : (key.modelData.label || "")
            color: key.modelData.kind === "accent" ? Kirigami.Theme.highlightedTextColor : key.kb.textColor
            opacity: key.modelData.kind === "space" ? 0.55 : (key.modelData.kind === "fn" ? 0.8 : 1)
            font.pixelSize: Math.max(5, Math.min(key.height, key.kb.unitW) * (key.modelData.kind === "char" ? 0.5 : 0.34) * key.kb.labelScale)
            font.weight: key.modelData.kind === "char" ? Font.Normal : Font.Medium
        }

        Text {
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.topMargin: Math.max(1, parent.height * 0.06)
            anchors.rightMargin: Math.max(1, parent.width * 0.1)
            visible: key.kb.showHints && key.modelData.kind === "char" && KeyboardModel.hints[key.modelData.id] !== undefined && height > 3
            text: KeyboardModel.hints[key.modelData.id] || ""
            color: key.kb.textColor
            opacity: 0.5
            font.pixelSize: Math.max(4, Math.min(key.height, key.kb.unitW) * 0.24 * key.kb.labelScale)
        }

        Kirigami.Icon {
            anchors.centerIn: parent
            visible: (key.modelData.icon || "").length > 0
            source: key.modelData.icon || ""
            width: Math.max(6, Math.min(key.height, key.kb.unitW) * 0.42 * key.kb.labelScale)
            height: width
            isMask: true
            color: key.modelData.kind === "accent" ? Kirigami.Theme.highlightedTextColor : key.kb.textColor
            opacity: 0.85
        }

        Rectangle {
            visible: key.kb.focusKey === key.modelData.id
            anchors.fill: parent
            anchors.margins: -Math.max(1.5, 2 * key.kb.metric) * key.kb.focusGlow
            radius: face.radius + Math.max(1.5, 2 * key.kb.metric)
            color: "transparent"
            border.width: Math.max(1.5, 2.2 * key.kb.metric)
            border.color: key.kb.accent
        }
    }
}
