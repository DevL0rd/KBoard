import QtQuick
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

RowLayout {
    id: status

    property string title
    property bool failed: false
    property bool pulsing: false
    property bool showDot: false
    property bool centered: false
    property int percent: -1
    property real phase: 0

    spacing: Kirigami.Units.smallSpacing

    Item {
        Layout.fillWidth: status.centered
    }

    Rectangle {
        Layout.alignment: Qt.AlignVCenter
        implicitWidth: Kirigami.Units.smallSpacing * 2
        implicitHeight: implicitWidth
        radius: width / 2
        visible: status.showDot
        color: status.failed ? VoiceStyle.danger : VoiceStyle.accent
        opacity: status.pulsing ? 0.55 + 0.45 * Math.abs(Math.sin(status.phase * Math.PI * 2)) : 1
    }

    Text {
        text: status.title
        color: status.failed ? VoiceStyle.danger : VoiceStyle.ink
        opacity: status.failed ? 1 : 0.72
        font.pixelSize: Kirigami.Theme.defaultFont.pixelSize * 1.05
        font.weight: Font.DemiBold
        font.letterSpacing: 0.3
    }

    Text {
        visible: status.percent >= 0
        text: i18nc("@info download percent", "%1%", status.percent)
        color: VoiceStyle.accent
        font.pixelSize: Kirigami.Theme.defaultFont.pixelSize * 1.05
        font.weight: Font.DemiBold
        font.features: { "tnum": 1 }
    }

    Item {
        Layout.fillWidth: true
    }
}
