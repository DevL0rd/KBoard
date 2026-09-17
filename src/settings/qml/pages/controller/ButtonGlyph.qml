pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

Rectangle {
    id: glyph

    property string text
    property bool active: false

    implicitWidth: Math.max(implicitHeight, label.implicitWidth + Kirigami.Units.largeSpacing)
    implicitHeight: Kirigami.Units.gridUnit * 1.3
    radius: implicitHeight / 2
    color: active ? Kirigami.Theme.highlightColor : Qt.alpha(Kirigami.Theme.textColor, 0.1)
    border.color: Qt.alpha(Kirigami.Theme.textColor, 0.25)

    QQC2.Label {
        id: label
        anchors.centerIn: parent
        text: glyph.text
        font.weight: Font.DemiBold
        color: glyph.active ? Kirigami.Theme.highlightedTextColor : Kirigami.Theme.textColor
    }
}
