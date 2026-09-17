pragma ComponentBehavior: Bound

import QtQuick
import org.devl0rd.kboard.config
import org.devl0rd.kboard.settings

SettingsPage {
    preview: GlidePreview {}

    Section {
        title: "Glide typing"

        SwitchRow {
            label: "Glide typing"
            description: "Slide your finger through the letters of a word instead of tapping each one"
            iconName: "draw-freehand"
            setting: "glideTyping"
        }

        SwitchRow {
            label: "Show the glide trail"
            description: "Draw a line behind your finger while you glide"
            iconName: "draw-path"
            setting: "glideTrail"
            enabled: Settings.glideTyping
        }
    }

    Section {
        title: "Gestures"

        SwitchRow {
            label: "Move the cursor with the space bar"
            description: "Slide left or right on space to move the text cursor precisely"
            iconName: "transform-move-horizontal"
            setting: "spaceCursorSwipe"
        }

        SwitchRow {
            label: "Delete words by sliding from backspace"
            description: "Slide left from backspace to select whole words, then lift to delete them"
            iconName: "edit-clear"
            setting: "backspaceSwipeDelete"
        }
    }
}
