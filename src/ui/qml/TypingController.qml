pragma ComponentBehavior: Bound

import QtQuick
import org.devl0rd.kboard.config
import org.devl0rd.kboard.platform
import org.devl0rd.kboard.typing
import org.devl0rd.kboard.sound
import org.devl0rd.kboard.emoji

QtObject {
    id: controller

    required property KeyboardView view
    required property SuggestionStrip strip
    property bool learningAllowed: true

    property var lastCorrection: null
    property bool lastAutoSpace: false
    property bool lastSpace: false
    property var glide: null
    property int selectedWords: 0

    readonly property FieldProfile field: FieldProfile {}
    readonly property TextEditing text: TextEditing {}
    readonly property bool smart: field.smart && view.pageId === "letters"
    readonly property string autocorrection: smart ? TypingEngine.autocorrection : ""
    readonly property var suggestions: glide ? [glide.candidates[1] ?? "", glide.candidates[0] ?? "", glide.candidates[2] ?? ""] : TypingEngine.suggestions
    readonly property bool showSuggestions: field.smart && Settings.suggestions && suggestions.some(entry => entry !== "")
    readonly property var emojis: Settings.emojiSuggestions && field.smart && TypingEngine.currentWord !== "" ? EmojiStore.suggestionsFor(TypingEngine.currentWord) : []
    readonly property var soundKinds: ({ space: "space", backspace: "backspace", enter: "enter", char: "key", key: "key", arrow: "key" })
    readonly property string punctuation: ".,!?;:"

    function learn(word) {
        if (word !== "" && learningAllowed && !InputContext.sensitive && Settings.learnWords)
            TypingEngine.acceptWord(word)
    }

    function resetState() {
        lastCorrection = null
        lastAutoSpace = false
        lastSpace = false
        glide = null
    }

    function correctWord() {
        const word = text.trailingWord
        if (!smart || word === "")
            return
        const expansion = TypingEngine.expansionFor(word)
        const correction = expansion !== "" ? expansion : Settings.autocorrect > 0 ? TypingEngine.autocorrection : ""
        if (correction !== "" && correction !== word) {
            text.replaceBefore(word.length, correction)
            lastCorrection = { original: word, replacement: correction }
            strip.flashCorrection(correction)
        } else {
            learn(word)
        }
    }

    function typeText(value) {
        glide = null
        if (punctuation.includes(value) && value.length === 1) {
            typePunctuation(value)
            return
        }
        lastCorrection = null
        lastAutoSpace = false
        lastSpace = false
        text.commit(value)
    }

    function typePunctuation(mark) {
        correctWord()
        if (Settings.smartPunctuation && lastAutoSpace && text.endsWith(" ")) {
            text.replaceBefore(1, mark + " ")
            lastAutoSpace = true
        } else {
            text.commit(mark)
            lastAutoSpace = false
        }
        lastSpace = false
    }

    function typeSpace() {
        glide = null
        if (Settings.doubleSpacePeriod && lastSpace && text.endsWith(" ") && text.isWordChar(text.before.charAt(text.before.length - 2))) {
            text.replaceBefore(1, ". ")
            lastSpace = false
            lastAutoSpace = true
            return
        }
        lastCorrection = null
        correctWord()
        text.commit(" ")
        lastSpace = true
        lastAutoSpace = false
    }

    function typeBackspace() {
        glide = null
        const correction = lastCorrection
        lastCorrection = null
        lastSpace = false
        lastAutoSpace = false
        if (correction && text.endsWith(correction.replacement + " ")) {
            text.replaceBefore(correction.replacement.length + 1, correction.original)
            TypingEngine.revertCorrection()
            TypingEngine.learn(correction.original)
            return
        }
        text.backspace()
    }

    function pickSuggestion(value) {
        if (glide) {
            text.replaceBefore(glide.word.length + 1, value + " ")
            glide = Object.assign({}, glide, { word: value })
        } else {
            const before = TypingEngine.wordCharsBefore
            if (TypingEngine.wordCharsAfter > 0)
                InputContext.deleteSurrounding(0, TypingEngine.wordCharsAfter)
            text.replaceBefore(before, value + " ")
        }
        learn(value)
        lastCorrection = null
        lastAutoSpace = true
        lastSpace = false
    }

    function commitGlide(candidates) {
        if (candidates.length === 0)
            return
        const shifted = view.shifted
        const words = candidates.map(word => shifted ? word.charAt(0).toUpperCase() + word.slice(1) : word)
        const separator = text.before !== "" && !/\s$/.test(text.before) ? " " : ""
        text.commit(separator + words[0] + " ")
        view.modifiers.consume()
        glide = { word: words[0], candidates: words }
        lastAutoSpace = true
        lastSpace = false
    }

    function selectWords(words) {
        const delta = words - selectedWords
        const code = KeyNames.evdev(delta > 0 ? "left" : "right")
        for (let i = 0; i < Math.abs(delta); ++i)
            InputContext.key(code, InputContext.ControlModifier | InputContext.ShiftModifier)
        selectedWords = words
    }

    function playFor(key) {
        KeySound.play(soundKinds[key.type] ?? "modifier")
    }

    property Connections inputContext: Connections {
        target: InputContext

        function onSurroundingTextChanged() {
            controller.text.sync()
        }

        function onActivated() {
            controller.resetState()
            controller.text.reset()
            controller.view.modifiers.reset()
            controller.view.pageId = controller.field.page
        }

        function onContentTypeChanged() {
            controller.view.pageId = controller.field.page
        }
    }

    property Connections keys: Connections {
        target: controller.view

        function onKeyPressed(key) {
            controller.playFor(key)
        }

        function onPopupOpened() {
            KeySound.play("popup")
        }

        function onTextKey(value) {
            controller.typeText(value)
        }

        function onActionKey(action, key) {
            if (action === "space")
                controller.typeSpace()
            else if (action === "backspace")
                controller.typeBackspace()
            else if (action === "enter")
                InputContext.enter()
            else if (action === "newline")
                controller.text.commit("\n")
        }

        function onShortcutKey(code, modifiers) {
            InputContext.key(code, modifiers)
        }

        function onBackspaceRepeated(count) {
            KeySound.play("backspace")
            if (count > 14)
                InputContext.deleteWordBefore()
            else
                controller.typeBackspace()
        }

        function onCursorSwipe(steps) {
            InputContext.moveCursor(steps)
        }

        function onDeleteSwipe(words) {
            controller.selectWords(words)
        }

        function onDeleteSwipeFinished(words) {
            if (words > 0)
                controller.text.backspace()
            controller.selectedWords = 0
        }

        function onGlideFinished(points) {
            TypingEngine.decodeGlideAsync(points)
        }
    }

    property Connections engine: Connections {
        target: TypingEngine

        function onGlideDecoded(candidates) {
            controller.commitGlide(candidates)
        }
    }

    property Connections suggestionStrip: Connections {
        target: controller.strip

        function onSuggestionPicked(value, index) {
            KeySound.play("key")
            controller.pickSuggestion(value)
        }

        function onSuggestionHeld(value) {
            if (TypingEngine.isLearned(value))
                TypingEngine.forget(value)
        }

        function onEmojiPicked(emoji) {
            KeySound.play("key")
            controller.text.commit(emoji)
            EmojiStore.recordUse(emoji)
        }
    }

    property Connections geometry: Connections {
        target: controller.view.surface

        function onGeometryUpdated() {
            const keys = controller.view.glideKeys()
            if (controller.view.pageId === "letters" && keys.length > 0)
                TypingEngine.setLayout(keys)
        }
    }

    property Binding autoShift: Binding {
        target: controller.view.modifiers
        property: "autoShift"
        value: controller.smart && Settings.autoCapitalize && TypingEngine.shouldCapitalize
    }
}
