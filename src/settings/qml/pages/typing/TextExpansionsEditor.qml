pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.devl0rd.kboard.config
import org.devl0rd.kboard.settings

SettingRow {
    id: row

    readonly property var entries: (SettingsStore.revision >= 0 ? Settings.textExpansions : []).map(entry => {
        const split = entry.indexOf("=");
        return split > 0 ? { shortcut: entry.substring(0, split), phrase: entry.substring(split + 1) } : { shortcut: entry, phrase: "" };
    })

    function store(list) {
        SettingsStore.setValue("textExpansions", list.map(entry => entry.shortcut + "=" + entry.phrase));
    }

    function replaced(index, change) {
        return entries.map((entry, i) => i === index ? Object.assign({}, entry, change) : entry);
    }

    wideControl: true

    ColumnLayout {
        width: parent.width
        spacing: Kirigami.Units.smallSpacing

        Repeater {
            model: row.entries

            RowLayout {
                id: entry
                required property var modelData
                required property int index
                Layout.fillWidth: true
                spacing: Kirigami.Units.smallSpacing

                QQC2.TextField {
                    text: entry.modelData.shortcut
                    placeholderText: "Shortcut"
                    validator: RegularExpressionValidator {
                        regularExpression: /[^\s=,]+/
                    }
                    Layout.preferredWidth: Kirigami.Units.gridUnit * 7
                    onEditingFinished: {
                        if (text !== entry.modelData.shortcut && text.length > 0) {
                            row.store(row.replaced(entry.index, { shortcut: text }));
                        }
                    }
                    Accessible.name: "Shortcut"
                }

                Kirigami.Icon {
                    source: "go-next-symbolic"
                    Layout.preferredWidth: Kirigami.Units.iconSizes.small
                    Layout.preferredHeight: Kirigami.Units.iconSizes.small
                    opacity: 0.6
                }

                QQC2.TextField {
                    text: entry.modelData.phrase
                    placeholderText: "Expands to"
                    validator: RegularExpressionValidator {
                        regularExpression: /[^,]*/
                    }
                    Layout.fillWidth: true
                    onEditingFinished: {
                        if (text !== entry.modelData.phrase) {
                            row.store(row.replaced(entry.index, { phrase: text }));
                        }
                    }
                    Accessible.name: "Expands to"
                }

                QQC2.ToolButton {
                    icon.name: "list-remove"
                    text: "Remove shortcut"
                    display: QQC2.AbstractButton.IconOnly
                    onClicked: row.store(row.entries.filter((_, i) => i !== entry.index))
                }
            }
        }

        RowLayout {
            spacing: Kirigami.Units.smallSpacing

            QQC2.TextField {
                id: newShortcut
                placeholderText: "New shortcut, like brb"
                validator: RegularExpressionValidator {
                    regularExpression: /[^\s=,]*/
                }
                Layout.preferredWidth: Kirigami.Units.gridUnit * 7
            }

            QQC2.TextField {
                id: newPhrase
                placeholderText: "Expands to"
                validator: RegularExpressionValidator {
                    regularExpression: /[^,]*/
                }
                Layout.fillWidth: true
                onAccepted: add.clicked()
            }

            QQC2.Button {
                id: add
                icon.name: "list-add"
                text: "Add"
                enabled: newShortcut.text.length > 0 && newPhrase.text.length > 0 && !row.entries.some(entry => entry.shortcut === newShortcut.text)
                onClicked: {
                    row.store(row.entries.concat([{ shortcut: newShortcut.text, phrase: newPhrase.text }]));
                    newShortcut.clear();
                    newPhrase.clear();
                }
            }
        }
    }
}
