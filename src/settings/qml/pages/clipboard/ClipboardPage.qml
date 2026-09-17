pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.devl0rd.kboard.config
import org.devl0rd.kboard.settings
import "ExpiryFormat.js" as ExpiryFormat

SettingsPage {
    id: page

    readonly property var history: Modules.clipboard ? Modules.clipboard.api : null

    preview: ClipboardPreview {}

    ModuleNotice {
        module: "clipboard"
        what: "Clipboard history"
    }

    Kirigami.InlineMessage {
        Layout.fillWidth: true
        Layout.maximumWidth: Kirigami.Units.gridUnit * 46
        Layout.alignment: Qt.AlignHCenter
        type: Kirigami.MessageType.Warning
        visible: page.history !== null && page.history.errorString.length > 0
        text: page.history ? page.history.errorString : ""
    }

    Section {
        title: "History"

        SwitchRow {
            label: "Clipboard history"
            description: "Remember what you copy so you can paste it again from the keyboard. Passwords from password managers are never kept."
            iconName: "edit-paste"
            setting: "clipboardEnabled"
        }

        SwitchRow {
            label: "Pause history"
            description: "Stop saving new copies for now without losing what's already there"
            iconName: "media-playback-pause"
            setting: "clipboardPaused"
            enabled: Settings.clipboardEnabled
        }

        ComboRow {
            label: "Keep items for"
            description: "Pinned items stay until you unpin them"
            iconName: "chronometer"
            setting: "clipboardExpiryMinutes"
            enabled: Settings.clipboardEnabled
            options: ExpiryFormat.options(Settings.clipboardExpiryMinutes)
        }

        SliderRow {
            label: "Most items to keep"
            iconName: "view-list-details"
            setting: "clipboardMaxItems"
            enabled: Settings.clipboardEnabled
            from: 5
            to: 500
            stepSize: 5
        }

        SwitchRow {
            label: "Keep history after restart"
            description: "Save the history to disk. Off keeps it in memory only."
            iconName: "document-save"
            setting: "clipboardPersist"
            enabled: Settings.clipboardEnabled
        }

        SwitchRow {
            label: "Include Klipper history"
            description: "Also show items from Plasma's clipboard applet when it's running"
            iconName: "klipper"
            setting: "clipboardImportKlipper"
            enabled: Settings.clipboardEnabled
        }
    }

    Section {
        title: "Suggestions"

        SwitchRow {
            label: "Paste chip for fresh copies"
            description: "Right after you copy something, offer it in the suggestion strip"
            iconName: "edit-paste"
            setting: "clipboardSuggestChip"
            enabled: Settings.clipboardEnabled
        }
    }
}
