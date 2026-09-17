pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.devl0rd.kboard.config
import org.devl0rd.kboard.settings
import "AppRules.js" as AppRules

SettingRow {
    id: row

    readonly property var entries: AppRules.parse(SettingsStore.revision >= 0 ? Settings.appRules : [])

    function store(list) {
        SettingsStore.setValue("appRules", AppRules.serialize(list));
    }

    wideControl: true

    ColumnLayout {
        width: parent.width
        spacing: Kirigami.Units.smallSpacing

        QQC2.Label {
            visible: row.entries.length === 0
            text: "No app rules yet. The keyboard behaves the same in every app."
            opacity: 0.6
            Layout.fillWidth: true
        }

        Repeater {
            model: row.entries

            RowLayout {
                id: entry
                required property var modelData
                required property int index
                readonly property var app: AppList.find(modelData.appId)
                Layout.fillWidth: true
                spacing: Kirigami.Units.largeSpacing

                Kirigami.Icon {
                    source: entry.app.icon || "application-x-executable"
                    Layout.preferredWidth: Kirigami.Units.iconSizes.medium
                    Layout.preferredHeight: Kirigami.Units.iconSizes.medium
                }

                ColumnLayout {
                    spacing: 0
                    Layout.fillWidth: true

                    QQC2.Label {
                        text: entry.app.name || entry.modelData.appId
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                    }

                    QQC2.Label {
                        text: entry.modelData.appId
                        opacity: 0.6
                        font: Kirigami.Theme.smallFont
                    }
                }

                QQC2.ComboBox {
                    model: AppRules.rules
                    textRole: "label"
                    valueRole: "value"
                    currentIndex: AppRules.rules.findIndex(rule => rule.value === entry.modelData.rule)
                    displayText: currentIndex >= 0 ? currentText : AppRules.labelFor(entry.modelData.rule)
                    onActivated: row.store(row.entries.map((item, i) => i === entry.index ? { appId: item.appId, rule: currentValue } : item))
                    Accessible.name: "Rule for " + (entry.app.name || entry.modelData.appId)
                }

                QQC2.ToolButton {
                    icon.name: "list-remove"
                    text: "Remove rule"
                    display: QQC2.AbstractButton.IconOnly
                    onClicked: row.store(row.entries.filter((_, i) => i !== entry.index))
                }
            }
        }

        RowLayout {
            spacing: Kirigami.Units.smallSpacing
            Layout.topMargin: Kirigami.Units.smallSpacing

            QQC2.ComboBox {
                id: appPicker
                model: AppList.applications
                textRole: "name"
                valueRole: "appId"
                editable: true
                Layout.preferredWidth: Kirigami.Units.gridUnit * 13
                Accessible.name: "App"
            }

            QQC2.ComboBox {
                id: rulePicker
                model: AppRules.rules
                textRole: "label"
                valueRole: "value"
                Layout.fillWidth: true
                Accessible.name: "Rule"
            }

            QQC2.Button {
                icon.name: "list-add"
                text: "Add rule"
                enabled: appPicker.currentIndex >= 0 && !row.entries.some(item => item.appId === appPicker.currentValue && item.rule === rulePicker.currentValue)
                onClicked: row.store(row.entries.concat([{ appId: appPicker.currentValue, rule: rulePicker.currentValue }]))
            }
        }
    }
}
