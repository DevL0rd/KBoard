import QtQuick
import org.kde.kirigami as Kirigami
import org.devl0rd.kboard.config
import org.devl0rd.kboard.clipboard

Kirigami.PlaceholderMessage {
    id: placeholder

    property string filter: "all"

    readonly property var copy: ({
        all: { icon: "edit-paste", text: i18n("Copied text and images show up here"), explanation: i18n("Tap to paste, swipe to remove, long-press to pin.") },
        pinned: { icon: "pin", text: i18n("No pinned items"), explanation: i18n("Long-press an item to pin it. Pinned items never expire.") },
        images: { icon: "image-x-generic", text: i18n("No copied images"), explanation: i18n("Images you copy appear here as thumbnails.") },
        links: { icon: "link", text: i18n("No copied links"), explanation: i18n("Links you copy appear here with their site.") },
        disabled: { icon: "edit-paste", text: i18n("Clipboard history is off"), explanation: i18n("Turn it on to keep what you copy close at hand.") }
    })
    readonly property var current: copy[ClipboardHistory.enabled ? filter : "disabled"]

    icon.name: current.icon
    text: current.text
    explanation: ClipboardHistory.enabled && ClipboardHistory.paused && filter === "all" ? i18n("Saving is paused. Resume to collect new copies.") : current.explanation
    helpfulAction: ClipboardHistory.enabled ? null : enableAction

    Kirigami.Action {
        id: enableAction
        icon.name: "checkmark"
        text: i18n("Turn on")
        onTriggered: {
            Settings.clipboardEnabled = true
            SettingsWatcher.save()
        }
    }
}
