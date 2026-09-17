pragma ComponentBehavior: Bound

import QtQuick
import org.devl0rd.kboard.config
import org.devl0rd.kboard.settings

SettingsPage {
    preview: EmojiPreview {}

    ModuleNotice {
        module: "emoji"
        what: "Emoji settings"
    }

    Section {
        title: "Emoji"

        SkinTonePicker {
            label: "Default skin tone"
            description: "Used for people and hands. Long-press an emoji to pick a different tone once."
            iconName: "face-smile"
            setting: "skinTone"
        }

        SliderRow {
            label: "Recent emoji to keep"
            description: "How many recently used emoji stay at the front of the panel"
            iconName: "document-open-recent"
            setting: "emojiRecentsLimit"
            from: 8
            to: 120
            stepSize: 4
        }
    }

    Section {
        title: "GIFs"

        GifStatusRow {
            label: "GIF provider"
        }

        SwitchRow {
            label: "GIF search"
            description: "Show the GIF panel on the keyboard"
            iconName: "image-gif"
            setting: "gifEnabled"
        }

        ChoiceRow {
            label: "Safety filter"
            description: "Hide GIFs that aren't suitable for everyone"
            iconName: "security-medium"
            setting: "gifContentFilter"
            enabled: Settings.gifEnabled
            options: [
                { value: 0, label: "Off" },
                { value: 1, label: "Low" },
                { value: 2, label: "Medium" },
                { value: 3, label: "High" }
            ]
        }

        ChoiceRow {
            label: "Insert GIFs as"
            description: "Some apps can't accept pasted images. Links work everywhere."
            iconName: "insert-image"
            setting: "gifInsertMode"
            enabled: Settings.gifEnabled
            options: [
                { value: 0, label: "Image" },
                { value: 1, label: "Link" }
            ]
        }

        SwitchRow {
            label: "Play GIFs in the panel"
            description: "Turn off to show still frames and save data"
            iconName: "media-playback-start"
            setting: "gifAutoplay"
            enabled: Settings.gifEnabled
        }

        SliderRow {
            label: "GIF cache size"
            description: "Space on disk for recently viewed GIFs"
            iconName: "drive-harddisk"
            setting: "gifCacheSizeMb"
            enabled: Settings.gifEnabled
            from: 8
            to: 1024
            stepSize: 8
            unit: "MB"
        }
    }
}
