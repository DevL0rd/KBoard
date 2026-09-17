pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Shapes
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.devl0rd.kboard.config
import org.devl0rd.kboard.settings
import "Ease.js" as Ease

Item {
    id: preview

    readonly property color accent: Settings.accentMode === 1 ? Settings.accentColor : Kirigami.Theme.highlightColor
    readonly property var sources: [
        { icon: "input-keyboard-virtual", label: "Words you type", on: Settings.learnWords },
        { icon: "edit-paste", label: "Clipboard", on: Settings.clipboardEnabled },
        { icon: "audio-input-microphone", label: "Voice", on: Settings.voiceEnabled }
    ]
    readonly property point hub: Qt.point(width * 0.55, height * 0.45)
    readonly property point cloud: Qt.point(width * 0.88, height * 0.45)
    readonly property string caption: "Learned words, clipboard history and speech never leave this computer" + (Settings.gifEnabled ? " · only GIF searches go to KLIPY" : "")

    SceneClock {
        id: clock
        steps: [{ id: "flow", ms: 2400 }]
    }

    Repeater {
        model: preview.sources

        Item {
            id: source
            required property var modelData
            required property int index
            readonly property point origin: Qt.point(preview.width * 0.14, preview.height * (0.18 + 0.3 * index))

            Shape {
                width: preview.width
                height: preview.height
                opacity: source.modelData.on ? 1 : 0.25
                preferredRendererType: Shape.CurveRenderer

                ShapePath {
                    strokeColor: Qt.alpha(Kirigami.Theme.textColor, 0.25)
                    strokeWidth: 1.5
                    fillColor: "transparent"
                    startX: source.origin.x + Kirigami.Units.gridUnit
                    startY: source.origin.y
                    PathCubic {
                        x: preview.hub.x - Kirigami.Units.gridUnit * 1.6
                        y: preview.hub.y
                        control1X: preview.width * 0.35
                        control1Y: source.origin.y
                        control2X: preview.width * 0.35
                        control2Y: preview.hub.y
                    }
                }
            }

            Rectangle {
                readonly property real t: ((clock.now / clock.total) + source.index * 0.33) % 1
                readonly property real e: Ease.inOutCubic(t)
                visible: source.modelData.on
                width: Kirigami.Units.smallSpacing * 2
                height: width
                radius: width / 2
                color: preview.accent
                x: Ease.lerp(source.origin.x + Kirigami.Units.gridUnit, preview.hub.x - Kirigami.Units.gridUnit * 1.6, e) - width / 2
                y: Ease.lerp(source.origin.y, preview.hub.y, Ease.inOutQuad(t)) - height / 2
                opacity: Ease.pulse(t)
            }

            Kirigami.Icon {
                x: source.origin.x - width / 2
                y: source.origin.y - height / 2
                width: Kirigami.Units.iconSizes.medium
                height: width
                source: source.modelData.icon
                opacity: source.modelData.on ? 1 : 0.4
            }
        }
    }

    Rectangle {
        x: preview.hub.x - width / 2
        y: preview.hub.y - height / 2
        width: Kirigami.Units.gridUnit * 3.4
        height: width
        radius: width / 2
        color: Qt.alpha(preview.accent, 0.15)
        border.color: preview.accent
        border.width: 2

        Kirigami.Icon {
            anchors.centerIn: parent
            width: parent.width * 0.55
            height: width
            source: "computer"
        }

        Kirigami.Icon {
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            width: Kirigami.Units.iconSizes.small
            height: width
            source: "object-locked"
        }
    }

    QQC2.Label {
        x: internet.x + internet.width / 2 - width / 2
        y: internet.y + internet.height + Kirigami.Units.smallSpacing
        text: "The internet"
        opacity: 0.7
        font: Kirigami.Theme.smallFont
    }

    QQC2.Label {
        x: preview.hub.x - width / 2
        y: preview.hub.y + Kirigami.Units.gridUnit * 2
        text: "This computer"
        font.weight: Font.DemiBold
    }

    Item {
        id: internet
        x: preview.cloud.x - width / 2
        y: preview.cloud.y - height / 2
        width: Kirigami.Units.iconSizes.large
        height: width
        opacity: 0.75

        Kirigami.Icon {
            anchors.fill: parent
            source: "cloudstatus"
        }

        Rectangle {
            anchors.centerIn: parent
            width: parent.width * 1.2
            height: 3
            rotation: -35
            radius: 1.5
            color: Kirigami.Theme.negativeTextColor
        }
    }
}
