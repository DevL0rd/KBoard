pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

RowLayout {
    id: meter

    property string label
    property real value: 0
    property color color: Kirigami.Theme.highlightColor

    spacing: Kirigami.Units.smallSpacing

    QQC2.Label {
        text: meter.label
        font: Kirigami.Theme.smallFont
        opacity: 0.7
        Layout.preferredWidth: Kirigami.Units.gridUnit * 3.5
    }

    Rectangle {
        Layout.fillWidth: true
        Layout.preferredHeight: 4
        radius: 2
        color: Qt.alpha(Kirigami.Theme.textColor, 0.12)

        Rectangle {
            width: parent.width * Math.max(0, Math.min(1, meter.value))
            height: parent.height
            radius: 2
            color: meter.color
        }
    }
}
