pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.devl0rd.kboard.settings

Item {
    id: header

    property var page: ({})
    property string status: "Changes apply instantly"
    property bool fresh: false

    implicitHeight: Kirigami.Units.gridUnit * 3.2

    Connections {
        target: SettingsStore
        function onApplied(message) {
            header.status = message;
            header.fresh = true;
            settle.restart();
        }
    }

    Timer {
        id: settle
        interval: 1800
        onTriggered: {
            header.fresh = false;
            header.status = "Changes apply instantly";
        }
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: Kirigami.Units.largeSpacing * 2
        anchors.rightMargin: Kirigami.Units.largeSpacing
        spacing: Kirigami.Units.largeSpacing

        Kirigami.Icon {
            source: header.page.icon || ""
            Layout.preferredWidth: Kirigami.Units.iconSizes.smallMedium
            Layout.preferredHeight: Kirigami.Units.iconSizes.smallMedium
        }

        ColumnLayout {
            spacing: 0
            Layout.fillWidth: true

            Kirigami.Heading {
                level: 2
                text: header.page.title || ""
                elide: Text.ElideRight
                Layout.fillWidth: true
            }

            QQC2.Label {
                text: header.page.description || ""
                elide: Text.ElideRight
                opacity: 0.7
                font: Kirigami.Theme.smallFont
                Layout.fillWidth: true
            }
        }

        RowLayout {
            spacing: Kirigami.Units.smallSpacing

            Kirigami.Icon {
                source: "dialog-ok-apply"
                visible: header.fresh
                Layout.preferredWidth: Kirigami.Units.iconSizes.small
                Layout.preferredHeight: Kirigami.Units.iconSizes.small
                color: Kirigami.Theme.positiveTextColor
            }

            QQC2.Label {
                text: header.status
                opacity: header.fresh ? 1 : 0.65
                color: header.fresh ? Kirigami.Theme.positiveTextColor : Kirigami.Theme.textColor
                font: Kirigami.Theme.smallFont
                Accessible.role: Accessible.StatusBar

                Behavior on opacity {
                    NumberAnimation {
                        duration: Motion.short
                    }
                }
            }
        }

        QQC2.ToolButton {
            icon.name: "edit-undo"
            text: "Undo"
            enabled: SettingsStore.canUndo
            onClicked: SettingsStore.undo()
            QQC2.ToolTip.text: "Undo the last change (Ctrl+Z)"
            QQC2.ToolTip.visible: hovered
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
        }

        QQC2.ToolButton {
            icon.name: "edit-reset"
            text: "Defaults"
            enabled: SettingsStore.revision >= 0 && !SettingsStore.representsDefaults
            onClicked: SettingsStore.defaults()
            QQC2.ToolTip.text: "Put every KBoard setting back to its default. You can undo this."
            QQC2.ToolTip.visible: hovered
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
        }
    }

    Kirigami.Separator {
        anchors.bottom: parent.bottom
        width: parent.width
        opacity: 0
    }
}
