pragma ComponentBehavior: Bound

import QtQuick

KeyboardView {
    id: keys

    property var target

    signal keyTapped(var key)
    signal finished

    numberRow: false
    desktopRow: false
    glideEnabled: false
    variant: ""

    onKeyPressed: key => keys.keyTapped(key)
    onTextKey: value => target.appendSearch(value)
    onBackspaceRepeated: target.backspaceSearch()
    onActionKey: (action, key) => {
        if (action === "space")
            target.appendSearch(" ")
        else if (action === "backspace")
            target.backspaceSearch()
        else if (action === "enter" || action === "hide")
            keys.finished()
    }
}
