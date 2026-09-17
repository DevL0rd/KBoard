pragma ComponentBehavior: Bound

import QtQuick
import org.devl0rd.kboard.config
import org.devl0rd.kboard.settings

SettingsPage {
    preview: TypingPreview {}

    Section {
        title: "Suggestions"

        SwitchRow {
            label: "Show suggestions"
            description: "Word choices in a strip above the keys"
            iconName: "view-list-text"
            setting: "suggestions"
        }

        SwitchRow {
            label: "Predict the next word"
            description: "Suggest what usually comes next after you finish a word"
            iconName: "go-next-skip"
            setting: "nextWordPrediction"
            enabled: Settings.suggestions
        }

        SwitchRow {
            label: "Suggest emoji"
            description: "Offer a matching emoji for words like “pizza” or “love”"
            iconName: "face-smile"
            setting: "emojiSuggestions"
            enabled: Settings.suggestions
        }
    }

    Section {
        title: "Corrections"

        ChoiceRow {
            label: "Autocorrect"
            description: "How eagerly misspelled words are fixed when you press space. Backspace right after a fix undoes it."
            iconName: "tools-check-spelling"
            setting: "autocorrect"
            options: [
                { value: 0, label: "Off" },
                { value: 1, label: "Mild" },
                { value: 2, label: "Normal" },
                { value: 3, label: "Aggressive" }
            ]
        }
    }

    Section {
        title: "Capitals and punctuation"

        SwitchRow {
            label: "Capitalise sentences"
            description: "Turn on shift at the start of a sentence"
            iconName: "format-text-capitalize"
            setting: "autoCapitalize"
        }

        SwitchRow {
            label: "Double space for a full stop"
            description: "Tap space twice to end a sentence"
            iconName: "format-text-symbol"
            setting: "doubleSpacePeriod"
        }

        SwitchRow {
            label: "Smart punctuation"
            description: "Tidy spaces around punctuation and use curly quotes"
            iconName: "format-text-code"
            setting: "smartPunctuation"
        }

        SwitchRow {
            label: "Close brackets and quotes"
            description: "Typing ( or \" adds the closing one after the cursor"
            iconName: "code-context"
            setting: "autoPairBrackets"
        }
    }

    Section {
        title: "Text shortcuts"

        TextExpansionsEditor {
            label: "Text shortcuts"
            description: "Type a shortcut and press space to replace it with the full phrase"
            iconName: "insert-text"
            setting: "textExpansions"
        }
    }
}
