pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.devl0rd.kboard.settings

SettingRow {
    id: row

    readonly property var engine: Modules.typing ? Modules.typing.api : null
    readonly property var words: engine ? engine.learnedWords : []
    property string filter

    signal clearRequested

    wideControl: true

    ColumnLayout {
        width: parent.width
        spacing: Kirigami.Units.smallSpacing

        RowLayout {
            Layout.fillWidth: true
            spacing: Kirigami.Units.smallSpacing

            Kirigami.SearchField {
                Layout.fillWidth: true
                placeholderText: row.words.length + " learned words"
                onTextChanged: row.filter = text.toLowerCase()
                enabled: row.words.length > 0
            }

            QQC2.Button {
                icon.name: "edit-clear-all"
                text: "Forget all"
                enabled: row.words.length > 0
                onClicked: row.clearRequested()
            }
        }

        Flow {
            Layout.fillWidth: true
            spacing: Kirigami.Units.smallSpacing

            Repeater {
                model: row.words.filter(word => row.filter.length === 0 || word.toLowerCase().indexOf(row.filter) >= 0).slice(0, 200)

                Kirigami.Chip {
                    required property string modelData
                    text: modelData
                    checkable: false
                    closable: true
                    onRemoved: row.engine.forget(modelData)
                    Accessible.description: "Forget " + modelData
                }
            }
        }

        QQC2.Label {
            visible: row.engine !== null && row.words.length === 0
            text: "KBoard hasn't learned any words yet"
            opacity: 0.6
        }
    }
}
