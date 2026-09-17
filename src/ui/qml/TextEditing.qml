pragma ComponentBehavior: Bound

import QtQuick
import org.devl0rd.kboard.platform
import org.devl0rd.kboard.typing

QtObject {
    id: editing

    property string before
    property string after
    property int pending: 0

    readonly property bool reportsText: InputContext.surroundingText !== ""
    readonly property string trailingWord: {
        let start = before.length
        while (start > 0 && isWordChar(before.charAt(start - 1)))
            --start
        return before.slice(start)
    }

    function isWordChar(ch) {
        return ch.toLowerCase() !== ch.toUpperCase() || /[0-9'’_-]/.test(ch)
    }

    function refresh() {
        TypingEngine.update(before, after)
    }

    function sync() {
        const real = InputContext.textBeforeCursor
        pending = Math.max(0, pending - 1)
        if (pending > 0 && before.startsWith(real) && before !== real)
            return
        pending = 0
        before = real
        after = InputContext.textAfterCursor
        refresh()
    }

    function reset() {
        pending = 0
        sync()
    }

    function commit(text) {
        InputContext.commit(text)
        pending += 1
        before += text
        refresh()
    }

    function deleteBefore(count) {
        if (count <= 0)
            return
        if (reportsText) {
            InputContext.deleteSurrounding(count, 0)
        } else {
            for (let i = 0; i < count; ++i)
                InputContext.backspace()
        }
        pending += 1
        before = before.slice(0, Math.max(0, before.length - count))
        refresh()
    }

    function replaceBefore(count, text) {
        deleteBefore(count)
        commit(text)
    }

    function backspace() {
        InputContext.backspace()
        pending += 1
        before = before.slice(0, -1)
        refresh()
    }

    function endsWith(text) {
        return before.endsWith(text)
    }
}
