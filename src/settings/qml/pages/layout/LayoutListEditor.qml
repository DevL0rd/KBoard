pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.devl0rd.kboard.config
import org.devl0rd.kboard.settings

SettingRow {
    id: row

    readonly property var layouts: SettingsStore.revision >= 0 ? Settings.layouts : []
    readonly property var catalog: Modules.layouts ? Modules.layouts.api : null
    readonly property var letterLayouts: catalog && catalog.revision >= 0 ? catalog.available : []
    readonly property var available: letterLayouts.filter(layout => layouts.indexOf(layout.id) < 0)

    function nameOf(id) {
        const info = catalog ? catalog.layout(id) : {};
        return info.name || id;
    }

    function moved(list, index, delta) {
        const next = list.slice();
        const [item] = next.splice(index, 1);
        next.splice(index + delta, 0, item);
        return next;
    }

    function remove(id) {
        const next = layouts.filter(entry => entry !== id);
        SettingsStore.setValues({ layouts: next, activeLayout: Settings.activeLayout === id ? next[0] : Settings.activeLayout });
    }

    wideControl: true
    extraSettings: ["activeLayout"]

    ColumnLayout {
        width: parent.width
        spacing: Kirigami.Units.smallSpacing

        ModuleNotice {
            module: "layouts"
            what: "The list of keyboard layouts"
        }

        Kirigami.InlineMessage {
            Layout.fillWidth: true
            type: Kirigami.MessageType.Error
            visible: row.catalog !== null && row.catalog.errorString.length > 0
            text: row.catalog ? row.catalog.errorString : ""
        }

        Repeater {
            model: row.layouts

            Rectangle {
                id: entry
                required property string modelData
                required property int index
                readonly property bool active: Settings.activeLayout === modelData
                readonly property bool known: row.catalog !== null && row.catalog.has(modelData)

                Layout.fillWidth: true
                implicitHeight: entryRow.implicitHeight + Kirigami.Units.smallSpacing * 2
                radius: Kirigami.Units.cornerRadius
                color: active ? Qt.alpha(Kirigami.Theme.highlightColor, 0.12) : Qt.alpha(Kirigami.Theme.textColor, 0.04)
                border.color: active ? Qt.alpha(Kirigami.Theme.highlightColor, 0.5) : "transparent"

                RowLayout {
                    id: entryRow
                    anchors.fill: parent
                    anchors.leftMargin: Kirigami.Units.largeSpacing
                    anchors.rightMargin: Kirigami.Units.smallSpacing
                    spacing: Kirigami.Units.smallSpacing

                    QQC2.RadioButton {
                        checked: entry.active
                        onClicked: SettingsStore.setValue("activeLayout", entry.modelData)
                        Accessible.name: "Use " + row.nameOf(entry.modelData)
                    }

                    ColumnLayout {
                        spacing: 0
                        Layout.fillWidth: true

                        QQC2.Label {
                            text: row.nameOf(entry.modelData)
                            font.weight: entry.active ? Font.DemiBold : Font.Normal
                            Layout.fillWidth: true
                            elide: Text.ElideRight
                        }

                        QQC2.Label {
                            text: entry.known ? entry.modelData : entry.modelData + " — this layout isn't installed"
                            color: entry.known ? Kirigami.Theme.textColor : Kirigami.Theme.negativeTextColor
                            opacity: entry.known ? 0.6 : 1
                            font: Kirigami.Theme.smallFont
                        }
                    }

                    QQC2.ToolButton {
                        icon.name: "go-up"
                        text: "Move up"
                        display: QQC2.AbstractButton.IconOnly
                        enabled: entry.index > 0
                        onClicked: SettingsStore.setValue("layouts", row.moved(row.layouts, entry.index, -1))
                    }

                    QQC2.ToolButton {
                        icon.name: "go-down"
                        text: "Move down"
                        display: QQC2.AbstractButton.IconOnly
                        enabled: entry.index < row.layouts.length - 1
                        onClicked: SettingsStore.setValue("layouts", row.moved(row.layouts, entry.index, 1))
                    }

                    QQC2.ToolButton {
                        icon.name: "list-remove"
                        text: "Remove"
                        display: QQC2.AbstractButton.IconOnly
                        enabled: row.layouts.length > 1
                        onClicked: row.remove(entry.modelData)
                    }
                }
            }
        }

        RowLayout {
            spacing: Kirigami.Units.smallSpacing
            visible: row.available.length > 0

            QQC2.ComboBox {
                id: picker
                model: row.available
                textRole: "name"
                valueRole: "id"
                Layout.preferredWidth: Kirigami.Units.gridUnit * 14
                Accessible.name: "Layout to add"
            }

            QQC2.Button {
                icon.name: "list-add"
                text: "Add layout"
                enabled: picker.currentIndex >= 0
                onClicked: SettingsStore.setValue("layouts", row.layouts.concat([picker.currentValue]))
            }
        }
    }
}
