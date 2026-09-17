import QtQuick
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Item {
    id: content

    property string text
    property string domain
    property real padding: Kirigami.Units.largeSpacing

    implicitHeight: column.implicitHeight + padding * 2

    ColumnLayout {
        id: column
        x: content.padding
        y: content.padding
        width: content.width - content.padding * 2
        spacing: Kirigami.Units.smallSpacing * 1.5

        RowLayout {
            Layout.fillWidth: true
            spacing: Kirigami.Units.smallSpacing * 2

            Rectangle {
                Layout.preferredWidth: Kirigami.Units.gridUnit * 1.6
                Layout.preferredHeight: Layout.preferredWidth
                radius: Kirigami.Units.cornerRadius * 1.5
                color: Qt.alpha(ClipboardStyle.accentColor, 0.18)

                Text {
                    anchors.centerIn: parent
                    text: content.domain.length > 0 ? content.domain.charAt(0).toUpperCase() : "?"
                    color: ClipboardStyle.accentColor
                    font.pixelSize: Math.round(parent.height * 0.5)
                    font.weight: Font.Bold
                }

                Kirigami.Icon {
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    anchors.margins: -2
                    width: Math.round(parent.width * 0.5)
                    height: width
                    source: "globe"
                    color: ClipboardStyle.accentColor
                    isMask: true
                }
            }

            Text {
                Layout.fillWidth: true
                text: content.domain
                elide: Text.ElideRight
                color: Kirigami.Theme.textColor
                font.pointSize: Kirigami.Theme.defaultFont.pointSize
                font.weight: Font.DemiBold
            }
        }

        Text {
            Layout.fillWidth: true
            text: content.text.replace(/^https?:\/\//i, "")
            textFormat: Text.PlainText
            wrapMode: Text.WrapAnywhere
            maximumLineCount: 2
            elide: Text.ElideRight
            color: Kirigami.Theme.linkColor
            font: Kirigami.Theme.smallFont
        }
    }
}
