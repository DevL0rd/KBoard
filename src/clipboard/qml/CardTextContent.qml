import QtQuick
import org.kde.kirigami as Kirigami

Item {
    id: content

    property string text
    property real padding: Kirigami.Units.largeSpacing
    property real trailingSpace: 0

    implicitHeight: label.implicitHeight + padding * 2

    Text {
        id: label
        x: content.padding
        y: content.padding
        width: content.width - content.padding * 2 - content.trailingSpace
        text: content.text
        textFormat: Text.PlainText
        wrapMode: Text.Wrap
        maximumLineCount: 4
        elide: Text.ElideRight
        color: Kirigami.Theme.textColor
        font.pointSize: Kirigami.Theme.defaultFont.pointSize
        lineHeight: 1.06
    }
}
