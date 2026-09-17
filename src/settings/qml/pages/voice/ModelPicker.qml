pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.devl0rd.kboard.settings

SettingRow {
    id: row

    readonly property var voice: Modules.voice ? Modules.voice.api : null

    wideControl: true

    ColumnLayout {
        width: parent.width
        spacing: Kirigami.Units.largeSpacing

        Kirigami.InlineMessage {
            Layout.fillWidth: true
            type: Kirigami.MessageType.Error
            visible: row.voice !== null && row.voice.downloadError.length > 0
            text: row.voice ? "Download failed: " + row.voice.downloadError : ""
        }

        GridLayout {
            Layout.fillWidth: true
            columns: Math.max(1, Math.floor(width / (Kirigami.Units.gridUnit * 13)))
            columnSpacing: Kirigami.Units.largeSpacing
            rowSpacing: Kirigami.Units.largeSpacing

            Repeater {
                model: row.voice ? row.voice.models : []

                ModelCard {
                    required property var modelData
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.preferredWidth: 1
                    entry: modelData
                    voice: row.voice
                }
            }
        }
    }
}
