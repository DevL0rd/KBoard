pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

Rectangle {
    id: chat

    property string emoji
    property bool linkMode: false
    property real arrival: 0
    property real phase: 0
    property bool animated: true

    radius: Kirigami.Units.cornerRadius * 2
    color: Qt.alpha(Kirigami.Theme.backgroundColor, 0.8)
    border.color: Qt.alpha(Kirigami.Theme.textColor, 0.15)

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Kirigami.Units.largeSpacing
        spacing: Kirigami.Units.smallSpacing

        Rectangle {
            Layout.preferredWidth: incoming.implicitWidth + Kirigami.Units.largeSpacing * 2
            Layout.preferredHeight: incoming.implicitHeight + Kirigami.Units.smallSpacing * 2
            radius: height / 2
            color: Qt.alpha(Kirigami.Theme.textColor, 0.1)

            QQC2.Label {
                id: incoming
                anchors.centerIn: parent
                text: "Did it work? " + chat.emoji
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            Rectangle {
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                width: chat.linkMode ? linkLabel.implicitWidth + Kirigami.Units.largeSpacing * 2 : Math.min(parent.width * 0.6, parent.height * 1.3)
                height: chat.linkMode ? linkLabel.implicitHeight + Kirigami.Units.smallSpacing * 2 : parent.height
                radius: Kirigami.Units.cornerRadius * 2
                color: Kirigami.Theme.highlightColor
                opacity: chat.arrival
                transform: Translate {
                    y: (1 - chat.arrival) * Kirigami.Units.gridUnit
                }

                GifTileMock {
                    anchors.fill: parent
                    anchors.margins: 3
                    visible: !chat.linkMode
                    seed: 4
                    animated: chat.animated
                    phase: chat.phase
                }

                QQC2.Label {
                    id: linkLabel
                    anchors.centerIn: parent
                    visible: chat.linkMode
                    text: "klipy.com/gifs/high-five"
                    color: Kirigami.Theme.highlightedTextColor
                    font.underline: true
                }
            }
        }
    }
}
