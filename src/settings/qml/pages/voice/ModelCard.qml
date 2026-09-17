pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.devl0rd.kboard.config
import org.devl0rd.kboard.settings
import "VoiceTiers.js" as VoiceTiers

Rectangle {
    id: card

    required property var entry
    required property var voice
    readonly property bool active: Settings.voiceModel === entry.id
    readonly property bool downloading: voice.downloadingId === entry.id

    implicitHeight: content.implicitHeight + Kirigami.Units.largeSpacing * 2
    radius: Kirigami.Units.cornerRadius * 2
    color: active ? Qt.alpha(Kirigami.Theme.highlightColor, 0.14) : Qt.alpha(Kirigami.Theme.textColor, 0.04)
    border.width: active ? 2 : 1
    border.color: active ? Kirigami.Theme.highlightColor : Qt.alpha(Kirigami.Theme.textColor, 0.14)

    ColumnLayout {
        id: content
        anchors.fill: parent
        anchors.margins: Kirigami.Units.largeSpacing
        spacing: Kirigami.Units.smallSpacing

        RowLayout {
            spacing: Kirigami.Units.smallSpacing
            Layout.fillWidth: true

            QQC2.Label {
                text: card.entry.name + " " + card.entry.variant
                font.weight: Font.DemiBold
                elide: Text.ElideRight
                Layout.fillWidth: true
            }

            Kirigami.Chip {
                visible: card.entry.recommended
                text: "Recommended"
                closable: false
                checkable: false
                icon.name: "starred-symbolic"
            }
        }

        QQC2.Label {
            text: card.entry.description
            wrapMode: Text.Wrap
            opacity: 0.75
            font: Kirigami.Theme.smallFont
            Layout.fillWidth: true
        }

        QQC2.Label {
            text: VoiceTiers.details(card.entry)
            opacity: 0.6
            font: Kirigami.Theme.smallFont
            Layout.fillWidth: true
            elide: Text.ElideRight
        }

        MeterBar {
            Layout.fillWidth: true
            label: "Speed"
            value: VoiceTiers.speed(card.entry)
        }

        MeterBar {
            Layout.fillWidth: true
            label: "Accuracy"
            value: VoiceTiers.accuracy(card.entry)
            color: Kirigami.Theme.positiveTextColor
        }

        RowLayout {
            spacing: Kirigami.Units.smallSpacing
            Layout.fillWidth: true
            Layout.topMargin: Kirigami.Units.smallSpacing

            QQC2.Button {
                visible: !card.entry.downloaded && !card.downloading
                icon.name: "download"
                text: "Download"
                enabled: card.voice.downloadingId.length === 0
                onClicked: card.voice.download(card.entry.id)
            }

            ProgressRing {
                visible: card.downloading
                value: card.voice.downloadProgress
                implicitWidth: Kirigami.Units.gridUnit * 1.8
            }

            QQC2.Button {
                visible: card.downloading
                icon.name: "dialog-cancel"
                text: "Cancel"
                onClicked: card.voice.cancelDownload()
            }

            QQC2.Button {
                visible: card.entry.downloaded
                icon.name: card.active ? "checkmark" : "go-next"
                text: card.active ? "In use" : "Use"
                checkable: true
                checked: card.active
                onClicked: SettingsStore.setValue("voiceModel", card.entry.id)
            }

            Item {
                Layout.fillWidth: true
            }

            QQC2.ToolButton {
                visible: card.entry.downloaded
                icon.name: "edit-delete"
                text: "Delete " + card.entry.name
                display: QQC2.AbstractButton.IconOnly
                onClicked: card.voice.deleteModel(card.entry.id)
                QQC2.ToolTip.text: "Delete the downloaded model to free " + VoiceTiers.sizeText(card.entry.sizeMb)
                QQC2.ToolTip.visible: hovered
                QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
            }
        }
    }
}
