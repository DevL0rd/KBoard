pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

Item {
    id: strip

    property var suggestions: []
    property string autocorrection
    property var emojis: []
    property bool showSuggestions: false
    property string activePanel: "keys"
    property bool split: false
    property real halfWidth: width
    property bool clipboardFresh: false
    property var clipboardEntry: ({})
    property bool swipeToHide: true
    property var tools: [
        { name: "emoji", glyph: "emoji" },
        { name: "full", icon: "input-keyboard" },
        { name: "split", icon: "view-split-left-right" },
        { name: "gif", text: "GIF" },
        { name: "clipboard", icon: "edit-paste" },
        { name: "voice", icon: "audio-input-microphone" },
        { name: "edit", icon: "edit-select-text" },
        { name: "settings", icon: "configure" }
    ]

    property bool toolbarPinned: false
    readonly property bool suggestionMode: showSuggestions && !toolbarPinned && activePanel === "keys"
    readonly property var slots: [suggestions[0] ?? "", suggestions[1] ?? "", suggestions[2] ?? ""]

    signal suggestionPicked(string text, int index)
    signal suggestionHeld(string text)
    signal emojiPicked(string emoji)
    signal toolRequested(string name)
    signal clipboardChipTapped
    signal clipboardChipDismissed
    signal swipedDown

    onSuggestionsChanged: toolbarPinned = false

    function suggestionRect(index) {
        const slot = slotRepeater.itemAt(index)
        if (!slot)
            return Qt.rect(0, 0, 0, 0)
        const point = slot.mapToItem(strip, 0, 0)
        return Qt.rect(point.x, point.y, slot.width, slot.height)
    }

    function flashCorrection(word) {
        flash.play(word)
    }

    Item {
        id: suggestionArea

        x: 0
        width: strip.split ? strip.halfWidth : strip.width
        height: parent.height
        opacity: strip.suggestionMode ? 1 : 0
        visible: opacity > 0
        scale: 0.97 + 0.03 * opacity

        Behavior on opacity {
            NumberAnimation { duration: Theme.duration(160); easing.type: Easing.OutCubic }
        }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: strip.height * 0.2
            anchors.rightMargin: strip.height * 0.2
            spacing: 0

            StripButton {
                Layout.fillHeight: true
                Layout.preferredWidth: strip.height
                glyph: "right"
                visible: !strip.split
                onClicked: strip.toolbarPinned = true
            }

            Repeater {
                id: slotRepeater

                model: 3

                delegate: RowLayout {
                    id: slot

                    required property int index

                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.preferredWidth: 1
                    spacing: 0

                    Rectangle {
                        Layout.preferredWidth: 1
                        Layout.preferredHeight: strip.height * 0.42
                        color: Theme.divider
                        visible: slot.index > 0
                    }

                    SuggestionCell {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        text: strip.slots[slot.index]
                        primary: slot.index === 1
                        corrected: strip.autocorrection !== "" && strip.autocorrection === text
                        onClicked: strip.suggestionPicked(text, slot.index)
                        onHeld: strip.suggestionHeld(text)
                    }
                }
            }

            Repeater {
                model: strip.emojis.slice(0, strip.split ? 1 : 2)

                delegate: StripButton {
                    required property string modelData

                    Layout.fillHeight: true
                    Layout.preferredWidth: strip.height
                    text: modelData
                    onClicked: strip.emojiPicked(modelData)
                }
            }
        }

        CorrectionFlash {
            id: flash

            anchors.centerIn: parent
            width: parent.width / 3
            height: parent.height
        }
    }

    Item {
        id: toolArea

        x: strip.split ? strip.width - strip.halfWidth : 0
        width: strip.split ? strip.halfWidth : strip.width
        height: parent.height
        opacity: strip.suggestionMode && !strip.split ? 0 : 1
        visible: opacity > 0

        Behavior on opacity {
            NumberAnimation { duration: Theme.duration(160); easing.type: Easing.OutCubic }
        }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: strip.height * 0.2
            anchors.rightMargin: strip.height * 0.2
            spacing: strip.height * 0.08

            Repeater {
                model: strip.tools

                delegate: StripButton {
                    required property var modelData

                    Layout.fillHeight: true
                    Layout.preferredWidth: strip.height * 1.15
                    iconName: modelData.icon ?? ""
                    glyph: modelData.glyph ?? ""
                    text: modelData.text ?? ""
                    active: strip.activePanel === modelData.name
                    onClicked: strip.toolRequested(modelData.name)
                }
            }

            ClipboardChip {
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.maximumWidth: strip.height * 7
                entry: strip.clipboardEntry
                shown: strip.clipboardFresh && strip.activePanel === "keys"
                onClicked: strip.clipboardChipTapped()
                onDismissed: strip.clipboardChipDismissed()
            }

            Item {
                Layout.fillWidth: true
            }

            StripButton {
                Layout.fillHeight: true
                Layout.preferredWidth: strip.height * 1.15
                glyph: strip.activePanel === "keys" ? "down" : ""
                iconName: strip.activePanel === "keys" ? "" : "input-keyboard-virtual"
                onClicked: strip.toolRequested(strip.activePanel === "keys" ? "hide" : "keys")
            }
        }
    }

    DragHandler {
        id: swipe

        target: null
        enabled: strip.swipeToHide
        yAxis.enabled: true
        xAxis.enabled: false
        onActiveChanged: {
            if (!active && translation.y > strip.height * 0.8)
                strip.swipedDown()
        }
    }
}
