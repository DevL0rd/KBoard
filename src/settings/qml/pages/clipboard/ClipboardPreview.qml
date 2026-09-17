pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.devl0rd.kboard.config
import org.devl0rd.kboard.settings
import "Ease.js" as Ease
import "ExpiryFormat.js" as ExpiryFormat

RowLayout {
    id: preview

    readonly property color accent: Settings.accentMode === 1 ? Settings.accentColor : Kirigami.Theme.highlightColor
    readonly property bool running: Settings.clipboardEnabled && !Settings.clipboardPaused
    readonly property real arrive: running ? clock.progress("arrive") : 0
    readonly property real expire: running ? clock.progress("expire") : 0
    readonly property real drain: clock.now / clock.total
    readonly property string retention: Settings.clipboardExpiryMinutes === 0 ? "until you remove them" : "for " + ExpiryFormat.text(Settings.clipboardExpiryMinutes)
    readonly property string caption: !Settings.clipboardEnabled ? "Clipboard history is off" : (Settings.clipboardPaused ? "History is paused, new copies aren't saved" : "Keeps up to " + Settings.clipboardMaxItems + " items " + retention)

    spacing: Kirigami.Units.gridUnit

    SceneClock {
        id: clock
        steps: [{ id: "pause", ms: 700 }, { id: "arrive", ms: 700, ease: "outCubic" }, { id: "hold", ms: 1800 }, { id: "expire", ms: 800, ease: "inOutCubic" }, { id: "rest", ms: 900 }]
    }

    Rectangle {
        Layout.fillHeight: true
        Layout.preferredWidth: Math.min(parent.width * 0.55, Kirigami.Units.gridUnit * 22)
        radius: Kirigami.Units.cornerRadius * 2
        color: Qt.alpha(Kirigami.Theme.backgroundColor, 0.85)
        border.color: Qt.alpha(Kirigami.Theme.textColor, 0.15)
        opacity: Settings.clipboardEnabled ? 1 : 0.45
        clip: true

        Column {
            id: list
            anchors.fill: parent
            anchors.margins: Kirigami.Units.largeSpacing
            spacing: Kirigami.Units.smallSpacing
            readonly property real rowHeight: (height - spacing * 3) / 4

            ClipItemMock {
                width: parent.width
                height: list.rowHeight
                text: "Meeting moved to 3 pm"
                icon: "view-calendar"
                pinned: true
                accent: preview.accent
            }

            Item {
                width: parent.width
                height: (list.rowHeight + list.spacing) * preview.arrive - (preview.arrive > 0 ? 0 : list.spacing)

                ClipItemMock {
                    width: parent.width
                    height: list.rowHeight
                    opacity: preview.arrive
                    scale: 0.9 + 0.1 * preview.arrive
                    text: "Hello from KBoard"
                    accent: preview.accent
                    expires: Settings.clipboardExpiryMinutes > 0
                    remaining: 1
                }
            }

            ClipItemMock {
                width: parent.width
                height: list.rowHeight
                text: "https://kde.org"
                icon: "internet-web-browser"
                accent: preview.accent
                expires: Settings.clipboardExpiryMinutes > 0
                remaining: 0.7 - 0.2 * preview.drain
            }

            ClipItemMock {
                width: parent.width
                height: list.rowHeight * (1 - preview.expire)
                opacity: 1 - preview.expire
                text: "Order 48213"
                icon: "view-barcode-qr"
                accent: preview.accent
                expires: Settings.clipboardExpiryMinutes > 0
                remaining: Math.max(0, 0.25 - 0.25 * Ease.segment(clock.now, 0, clock.starts["expire"]))
            }
        }
    }

    ColumnLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true
        spacing: Kirigami.Units.largeSpacing

        SuggestionBar {
            Layout.fillWidth: true
            accent: preview.accent
            chipText: Settings.clipboardSuggestChip && preview.arrive > 0.5 && clock.step !== "rest" ? "Paste “Hello from KBoard”" : ""
            suggestions: ["", "", ""]
        }

        MiniKeyboard {
            Layout.fillWidth: true
            Layout.fillHeight: true
            showHints: false
        }

        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: Kirigami.Units.smallSpacing
            visible: Settings.clipboardEnabled

            Kirigami.Icon {
                source: Settings.clipboardPersist ? "document-save" : "view-refresh"
                Layout.preferredWidth: Kirigami.Units.iconSizes.small
                Layout.preferredHeight: Kirigami.Units.iconSizes.small
            }

            QQC2.Label {
                text: Settings.clipboardPersist ? "Kept after restart" : "Cleared when you log out"
                font: Kirigami.Theme.smallFont
                opacity: 0.75
            }
        }
    }
}
