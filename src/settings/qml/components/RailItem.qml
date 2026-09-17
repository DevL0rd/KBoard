pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

QQC2.ItemDelegate {
    id: item

    property string title
    property string subtitle
    property string iconName
    property string query
    property bool selected
    property bool keyboardFocus

    function markup(text) {
        const escaped = text.replace(/&/g, "&amp;").replace(/</g, "&lt;");
        const words = query.trim().split(/\s+/).filter(Boolean).map(word => word.replace(/[.*+?^${}()|[\]\\]/g, "\\$&"));
        if (words.length === 0) {
            return escaped;
        }
        return escaped.replace(new RegExp("(" + words.join("|") + ")", "gi"), "<b>$1</b>");
    }

    focusPolicy: Qt.NoFocus
    hoverEnabled: true
    implicitHeight: subtitle.length > 0 ? Kirigami.Units.gridUnit * 2.6 : Kirigami.Units.gridUnit * 2.2
    padding: 0
    leftPadding: Kirigami.Units.largeSpacing
    rightPadding: Kirigami.Units.largeSpacing
    Accessible.name: title

    background: Rectangle {
        radius: Kirigami.Units.cornerRadius * 1.5
        color: item.selected ? Qt.alpha(Kirigami.Theme.highlightColor, 0.2)
                             : (item.hovered || item.keyboardFocus ? Qt.alpha(Kirigami.Theme.textColor, 0.07) : "transparent")
        border.width: item.keyboardFocus ? 1 : 0
        border.color: Qt.alpha(Kirigami.Theme.highlightColor, 0.8)

        Behavior on color {
            ColorAnimation {
                duration: Motion.quick
            }
        }
    }

    contentItem: RowLayout {
        spacing: Kirigami.Units.largeSpacing

        Kirigami.Icon {
            source: item.iconName
            Layout.preferredWidth: Kirigami.Units.iconSizes.small
            Layout.preferredHeight: Kirigami.Units.iconSizes.small
            opacity: item.selected ? 1 : 0.85
        }

        ColumnLayout {
            spacing: 0
            Layout.fillWidth: true

            QQC2.Label {
                text: item.query.length > 0 ? item.markup(item.title) : item.title
                textFormat: item.query.length > 0 ? Text.StyledText : Text.PlainText
                elide: Text.ElideRight
                font.weight: item.selected ? Font.DemiBold : Font.Normal
                Layout.fillWidth: true
            }

            QQC2.Label {
                text: item.subtitle
                visible: text.length > 0
                elide: Text.ElideRight
                opacity: 0.65
                font: Kirigami.Theme.smallFont
                Layout.fillWidth: true
            }
        }
    }
}
