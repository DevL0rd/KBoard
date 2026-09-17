pragma ComponentBehavior: Bound

import QtQuick
import org.devl0rd.kboard.emoji
import org.devl0rd.kboard.gif
import org.devl0rd.kboard.clipboard
import org.devl0rd.kboard.voice
import org.devl0rd.kboard.sound

Item {
    id: host

    property string panel: "keys"
    property real searchKeysHeight: height * 0.6
    readonly property var current: ({ emoji: emojiSlot.item, gif: gifSlot.item, clipboard: clipboardSlot.item, voice: voiceSlot.item, edit: editSlot.item })[panel] ?? null
    readonly property bool searching: current !== null && current.searchActive === true
    default property alias keysContent: keysSlot.data

    signal closeRequested

    function openSearch(item) {
        item.searchActive = true
    }

    function closeSearch() {
        if (host.current && typeof host.current.closeSearch === "function")
            host.current.closeSearch()
        else if (host.current)
            host.current.searchActive = false
    }

    PanelSlot {
        id: keysSlot

        anchors.fill: parent
        name: "keys"
        current: host.panel
    }

    Item {
        id: panels

        anchors.fill: parent
        anchors.bottomMargin: host.searching ? host.searchKeysHeight : 0

        Behavior on anchors.bottomMargin {
            NumberAnimation { duration: Theme.duration(200); easing.type: Easing.OutCubic }
        }

        PanelSlot {
            id: emojiSlot

            anchors.fill: parent
            name: "emoji"
            current: host.panel
            component: EmojiPanel {
                id: emojiPanel

                onCloseRequested: host.closeRequested()
                onSearchRequested: host.openSearch(emojiPanel)
                onEmojiCommitted: KeySound.play("key")
            }
        }

        PanelSlot {
            id: gifSlot

            anchors.fill: parent
            name: "gif"
            current: host.panel
            component: GifPanel {
                id: gifPanel

                onCloseRequested: host.closeRequested()
                onSearchRequested: host.openSearch(gifPanel)
            }
        }

        PanelSlot {
            id: clipboardSlot

            anchors.fill: parent
            name: "clipboard"
            current: host.panel
            component: ClipboardPanel {
                onCloseRequested: host.closeRequested()
                onPasted: host.closeRequested()
            }
        }

        PanelSlot {
            id: voiceSlot

            anchors.fill: parent
            name: "voice"
            current: host.panel
            component: VoicePanel {
                onCloseRequested: host.closeRequested()
            }
        }

        PanelSlot {
            id: editSlot

            anchors.fill: parent
            name: "edit"
            current: host.panel
            component: EditPanel {
                onCloseRequested: host.closeRequested()
                onKeyTapped: KeySound.play("key")
            }
        }
    }

    SearchKeys {
        id: searchKeys

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: host.searchKeysHeight
        target: host.current
        visible: opacity > 0
        opacity: host.searching ? 1 : 0
        transform: Translate {
            y: (1 - searchKeys.opacity) * host.searchKeysHeight * 0.3
        }
        onKeyTapped: key => KeySound.play(key.type === "backspace" ? "backspace" : key.type === "space" ? "space" : "key")
        onFinished: host.closeSearch()

        Behavior on opacity {
            NumberAnimation { duration: Theme.duration(200); easing.type: Easing.OutCubic }
        }
    }
}
