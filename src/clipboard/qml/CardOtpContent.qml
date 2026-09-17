import QtQuick
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Item {
    id: content

    property string text
    property string code
    property real padding: Kirigami.Units.largeSpacing

    implicitHeight: column.implicitHeight + padding * 2

    Rectangle {
        anchors.fill: parent
        radius: ClipboardStyle.cardRadius
        color: Qt.alpha(ClipboardStyle.accentColor, 0.1)
    }

    ColumnLayout {
        id: column
        x: content.padding
        y: content.padding
        width: content.width - content.padding * 2
        spacing: Kirigami.Units.smallSpacing

        RowLayout {
            spacing: Kirigami.Units.smallSpacing

            Kirigami.Icon {
                Layout.preferredWidth: Kirigami.Units.iconSizes.small
                Layout.preferredHeight: Layout.preferredWidth
                source: "lock"
                color: ClipboardStyle.accentColor
                isMask: true
            }

            Text {
                text: i18n("Verification code")
                color: ClipboardStyle.accentColor
                font.pointSize: Kirigami.Theme.smallFont.pointSize
                font.weight: Font.DemiBold
                font.capitalization: Font.AllUppercase
                font.letterSpacing: 0.6
            }
        }

        Text {
            Layout.fillWidth: true
            text: content.code.length === 6 ? content.code.slice(0, 3) + " " + content.code.slice(3) : content.code
            color: Kirigami.Theme.textColor
            font.family: "monospace"
            font.pointSize: Kirigami.Theme.defaultFont.pointSize * 2
            font.weight: Font.Bold
            font.letterSpacing: Kirigami.Units.smallSpacing
            fontSizeMode: Text.HorizontalFit
            minimumPointSize: Kirigami.Theme.defaultFont.pointSize
        }

        Text {
            Layout.fillWidth: true
            visible: content.text.replace(/\D/g, "") !== content.code
            text: content.text
            textFormat: Text.PlainText
            elide: Text.ElideRight
            maximumLineCount: 1
            color: Kirigami.Theme.disabledTextColor
            font: Kirigami.Theme.smallFont
        }
    }
}
