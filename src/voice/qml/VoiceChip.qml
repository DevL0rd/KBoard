import QtQuick
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Rectangle {
    id: chip

    property string iconName
    property string text
    property bool highlighted: false

    implicitHeight: Kirigami.Units.gridUnit * 1.6
    implicitWidth: row.implicitWidth + Kirigami.Units.largeSpacing * 2
    radius: height / 2
    color: highlighted ? Qt.alpha(VoiceStyle.accent, 0.16) : Qt.alpha(VoiceStyle.ink, 0.07)
    border.width: 1
    border.color: highlighted ? Qt.alpha(VoiceStyle.accent, 0.35) : Qt.alpha(VoiceStyle.ink, 0.08)

    RowLayout {
        id: row
        anchors.centerIn: parent
        spacing: Kirigami.Units.smallSpacing

        Kirigami.Icon {
            source: chip.iconName
            implicitWidth: Kirigami.Units.iconSizes.small
            implicitHeight: implicitWidth
            isMask: true
            color: chip.highlighted ? VoiceStyle.accent : VoiceStyle.ink
            opacity: chip.highlighted ? 1 : 0.7
        }

        Text {
            text: chip.text
            color: VoiceStyle.ink
            opacity: 0.85
            font.pixelSize: Kirigami.Theme.smallFont.pixelSize * 1.05
            font.weight: Font.Medium
        }
    }
}
