import QtQuick
import org.kde.kirigami as Kirigami

Text {
    property bool large: false
    property bool danger: false
    property bool centered: false

    horizontalAlignment: centered ? Text.AlignHCenter : Text.AlignLeft
    wrapMode: Text.WordWrap
    maximumLineCount: 4
    elide: Text.ElideRight
    color: danger ? VoiceStyle.danger : VoiceStyle.ink
    font.pixelSize: Kirigami.Theme.defaultFont.pixelSize * (large ? 1.55 : 1)
    font.weight: large ? Font.Light : Font.Normal
}
