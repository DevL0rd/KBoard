pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

Item {
    id: stage

    property alias sourceComponent: loader.sourceComponent
    readonly property alias item: loader.item
    readonly property string caption: captionOf(loader.item)

    function captionOf(scene: var): string {
        return scene && scene.caption !== undefined ? scene.caption : "";
    }

    implicitHeight: Kirigami.Units.gridUnit * 13

    Rectangle {
        id: frame
        anchors.fill: parent
        anchors.leftMargin: Kirigami.Units.largeSpacing
        anchors.rightMargin: Kirigami.Units.largeSpacing
        anchors.topMargin: Kirigami.Units.smallSpacing
        anchors.bottomMargin: Kirigami.Units.smallSpacing
        radius: Kirigami.Units.cornerRadius * 2
        clip: true
        gradient: Gradient {
            GradientStop {
                position: 0
                color: Qt.alpha(Kirigami.Theme.textColor, 0.075)
            }
            GradientStop {
                position: 1
                color: Qt.alpha(Kirigami.Theme.textColor, 0.03)
            }
        }
        border.color: Qt.alpha(Kirigami.Theme.textColor, 0.1)

        Loader {
            id: loader
            anchors.fill: parent
            anchors.leftMargin: Kirigami.Units.largeSpacing * 2
            anchors.rightMargin: Kirigami.Units.largeSpacing * 2
            anchors.topMargin: Kirigami.Units.largeSpacing * 2
            anchors.bottomMargin: stage.caption.length > 0 ? captionLabel.height + Kirigami.Units.largeSpacing * 2 : Kirigami.Units.largeSpacing * 2
        }

        QQC2.Label {
            id: captionLabel
            anchors.bottom: parent.bottom
            anchors.bottomMargin: Kirigami.Units.largeSpacing
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width - Kirigami.Units.gridUnit * 2
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideRight
            text: stage.caption
            visible: text.length > 0
            opacity: 0.75
            font: Kirigami.Theme.smallFont
        }
    }
}
