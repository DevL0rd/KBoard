pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

Item {
    id: bar

    property var suggestions: ["", "", ""]
    property int picked: -1
    property real pick: 0
    property string chipText
    property string chipIcon: "edit-paste"
    property color accent: Kirigami.Theme.highlightColor

    implicitHeight: Kirigami.Units.gridUnit * 1.6

    RowLayout {
        anchors.fill: parent
        spacing: 0
        visible: bar.chipText.length === 0

        Repeater {
            model: 3

            Item {
                required property int index
                Layout.fillWidth: true
                Layout.fillHeight: true

                Rectangle {
                    anchors.fill: parent
                    anchors.margins: 2
                    radius: height / 2
                    color: Qt.alpha(bar.accent, bar.picked === parent.index ? 0.35 * bar.pick : 0)
                }

                QQC2.Label {
                    anchors.centerIn: parent
                    text: bar.suggestions[parent.index] || ""
                    font.weight: parent.index === 1 ? Font.DemiBold : Font.Normal
                    opacity: parent.index === 1 ? 1 : 0.75
                    elide: Text.ElideRight
                    width: Math.min(implicitWidth, parent.width - 4)
                }

                Kirigami.Separator {
                    visible: parent.index < 2
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    height: parent.height * 0.5
                }
            }
        }
    }

    Rectangle {
        visible: bar.chipText.length > 0
        anchors.verticalCenter: parent.verticalCenter
        x: Kirigami.Units.smallSpacing
        height: parent.height - 4
        width: Math.min(parent.width - 8, chipRow.implicitWidth + Kirigami.Units.largeSpacing * 2)
        radius: height / 2
        color: Qt.alpha(bar.accent, 0.2)
        border.color: Qt.alpha(bar.accent, 0.6)

        RowLayout {
            id: chipRow
            anchors.centerIn: parent
            spacing: Kirigami.Units.smallSpacing

            Kirigami.Icon {
                source: bar.chipIcon
                Layout.preferredWidth: Kirigami.Units.iconSizes.small
                Layout.preferredHeight: Kirigami.Units.iconSizes.small
            }

            QQC2.Label {
                text: bar.chipText
                elide: Text.ElideRight
                Layout.maximumWidth: bar.width - Kirigami.Units.gridUnit * 3
            }
        }
    }
}
