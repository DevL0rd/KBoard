pragma ComponentBehavior: Bound

import QtQuick
import org.kde.kirigami as Kirigami
import "KeyStyle.js" as KeyStyle

Rectangle {
    id: sample

    property var choice: null
    property int keyStyle: choice ? choice.value : KeyStyle.Raised
    property color accent: Kirigami.Theme.highlightColor
    readonly property bool dark: Kirigami.ColorUtils.brightnessForColor(Kirigami.Theme.backgroundColor) === Kirigami.ColorUtils.Dark
    readonly property var keyPalette: ({ key: colors.Kirigami.Theme.backgroundColor, panel: Kirigami.Theme.backgroundColor, accent: accent, text: Kirigami.Theme.textColor, dark: dark })

    radius: Kirigami.Units.cornerRadius
    color: keyStyle === KeyStyle.Glass ? Qt.alpha(Qt.tint(Kirigami.Theme.backgroundColor, Qt.alpha(accent, 0.3)), 0.8) : Qt.darker(Kirigami.Theme.backgroundColor, 1.08)
    clip: true

    Item {
        id: colors
        Kirigami.Theme.colorSet: Kirigami.Theme.Button
        Kirigami.Theme.inherit: false
    }

    Row {
        anchors.centerIn: parent
        spacing: Math.max(3, sample.height * 0.08)

        Repeater {
            model: [{ label: "A", kind: "char" }, { label: "S", kind: "char" }, { label: "", kind: "accent" }]

            Item {
                required property var modelData
                width: sample.height * 0.5
                height: sample.height * 0.62

                Rectangle {
                    visible: sample.keyStyle === KeyStyle.Raised
                    anchors.fill: parent
                    anchors.topMargin: 2
                    anchors.bottomMargin: -2
                    radius: face.radius
                    color: KeyStyle.shadowColor(sample.keyPalette)
                }

                Rectangle {
                    id: face
                    anchors.fill: parent
                    radius: Math.max(2, height * 0.18)
                    color: KeyStyle.face(sample.keyStyle, parent.modelData.kind, sample.keyPalette)
                    border.width: KeyStyle.borderWidth(sample.keyStyle)
                    border.color: KeyStyle.borderColor(sample.keyStyle, sample.keyPalette)
                }

                Text {
                    anchors.centerIn: parent
                    text: parent.modelData.label
                    color: Kirigami.Theme.textColor
                    font.pixelSize: parent.height * 0.45
                }
            }
        }
    }
}
