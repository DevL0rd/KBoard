pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

Rectangle {
    id: item

    property string text
    property string icon: "edit-paste"
    property bool pinned: false
    property real remaining: 1
    property bool expires: true
    property color accent: Kirigami.Theme.highlightColor

    radius: Kirigami.Units.cornerRadius * 1.5
    color: pinned ? Qt.alpha(accent, 0.14) : Qt.alpha(Kirigami.Theme.textColor, 0.06)
    border.color: pinned ? Qt.alpha(accent, 0.5) : Qt.alpha(Kirigami.Theme.textColor, 0.1)
    clip: true

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: Kirigami.Units.largeSpacing
        anchors.rightMargin: Kirigami.Units.largeSpacing
        spacing: Kirigami.Units.smallSpacing

        Kirigami.Icon {
            source: item.icon
            Layout.preferredWidth: Kirigami.Units.iconSizes.small
            Layout.preferredHeight: Kirigami.Units.iconSizes.small
        }

        QQC2.Label {
            text: item.text
            elide: Text.ElideRight
            Layout.fillWidth: true
        }

        Kirigami.Icon {
            source: "pin"
            visible: item.pinned
            color: item.accent
            Layout.preferredWidth: Kirigami.Units.iconSizes.small
            Layout.preferredHeight: Kirigami.Units.iconSizes.small
        }
    }

    Rectangle {
        visible: item.expires && !item.pinned
        anchors.bottom: parent.bottom
        height: 2
        width: parent.width * item.remaining
        color: Qt.alpha(item.accent, 0.8)
    }
}
