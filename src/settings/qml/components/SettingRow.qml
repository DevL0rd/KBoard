pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.devl0rd.kboard.settings

FormCard.AbstractFormDelegate {
    id: row

    property string label
    property string description
    property string iconName
    property string setting
    property var extraSettings: []
    readonly property var resetNames: setting.length > 0 ? [setting].concat(extraSettings) : extraSettings
    readonly property var current: SettingsStore.revision >= 0 && setting.length > 0 ? SettingsStore.value(setting) : undefined
    default property alias control: controlSlot.data
    property bool wideControl: false
    property bool emphasized: false
    readonly property bool modified: SettingsStore.revision >= 0 && resetNames.some(name => !SettingsStore.isDefault(name))

    function apply(value) {
        SettingsStore.setValue(row.setting, value);
    }

    function reset() {
        for (const name of resetNames) {
            SettingsStore.resetToDefault(name);
        }
    }

    focusPolicy: Qt.NoFocus
    background: Rectangle {
        radius: Kirigami.Units.cornerRadius
        color: row.emphasized ? Qt.alpha(Kirigami.Theme.highlightColor, 0.08) : "transparent"
        Behavior on color {
            ColorAnimation {
                duration: Motion.short
            }
        }
    }
    Accessible.name: label
    Accessible.description: description

    contentItem: GridLayout {
        columns: row.wideControl ? 1 : 2
        columnSpacing: Kirigami.Units.gridUnit
        rowSpacing: Kirigami.Units.smallSpacing

        RowLayout {
            Layout.fillWidth: true
            spacing: Kirigami.Units.largeSpacing

            Kirigami.Icon {
                source: row.iconName
                visible: row.iconName.length > 0
                Layout.preferredWidth: Kirigami.Units.iconSizes.smallMedium
                Layout.preferredHeight: Kirigami.Units.iconSizes.smallMedium
                Layout.alignment: Qt.AlignTop
                Layout.topMargin: 1
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 1

                RowLayout {
                    spacing: Kirigami.Units.smallSpacing
                    Layout.fillWidth: true

                    QQC2.Label {
                        text: row.label
                        wrapMode: Text.Wrap
                        Layout.fillWidth: !badge.visible
                    }

                    ModifiedBadge {
                        id: badge
                        visible: row.modified
                        onResetRequested: row.reset()
                    }

                    Item {
                        visible: badge.visible
                        Layout.fillWidth: true
                    }
                }

                QQC2.Label {
                    text: row.description
                    visible: text.length > 0
                    wrapMode: Text.Wrap
                    opacity: 0.7
                    font: Kirigami.Theme.smallFont
                    Layout.fillWidth: true
                }
            }
        }

        Item {
            id: controlSlot
            implicitWidth: childrenRect.width
            implicitHeight: childrenRect.height
            Layout.alignment: row.wideControl ? Qt.AlignLeft : (Qt.AlignRight | Qt.AlignVCenter)
            Layout.fillWidth: row.wideControl
        }
    }
}
