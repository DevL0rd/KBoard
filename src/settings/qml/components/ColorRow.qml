pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Dialogs
import org.kde.kirigami as Kirigami

SettingRow {
    id: row

    property bool enabledControl: true

    QQC2.Button {
        id: swatch
        enabled: row.enabledControl
        implicitWidth: Kirigami.Units.gridUnit * 5
        text: row.current !== undefined ? String(row.current).toUpperCase() : ""
        onClicked: dialog.open()
        Accessible.name: row.label

        contentItem: RowLayout {
            spacing: Kirigami.Units.smallSpacing

            Rectangle {
                Layout.preferredWidth: Kirigami.Units.iconSizes.small
                Layout.preferredHeight: Kirigami.Units.iconSizes.small
                radius: height / 2
                color: row.current !== undefined ? row.current : "transparent"
                border.color: Qt.alpha(Kirigami.Theme.textColor, 0.3)
                opacity: swatch.enabled ? 1 : 0.4
            }

            QQC2.Label {
                text: swatch.text
                font.family: "monospace"
                opacity: swatch.enabled ? 1 : 0.5
                Layout.fillWidth: true
            }
        }
    }

    ColorDialog {
        id: dialog
        title: row.label
        selectedColor: row.current !== undefined ? row.current : "white"
        onAccepted: row.apply(selectedColor.toString())
    }
}
