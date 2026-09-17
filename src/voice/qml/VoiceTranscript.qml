pragma ComponentBehavior: Bound

import QtQuick
import org.kde.kirigami as Kirigami

Item {
    id: transcript

    property string committed
    property string partial
    property string placeholder
    property bool centered: false
    property real fontSize: Kirigami.Theme.defaultFont.pixelSize * 2

    implicitHeight: Math.max(fontSize * 1.4, flow.implicitHeight)
    clip: true

    function words(text, count) {
        const parts = text.replace(/\n/g, " ⏎ ").split(/\s+/).filter(word => word.length > 0)
        return parts.slice(Math.max(0, parts.length - count))
    }

    function wanted() {
        const done = words(committed, partial.length > 0 ? 10 : 18).map(word => ({ word: word, live: false }))
        return done.concat(words(partial, 60).map(word => ({ word: word, live: true })))
    }

    function dropLeading(target) {
        if (target.length === 0 || wordModel.count <= target.length)
            return
        for (let shift = 0; shift <= wordModel.count - target.length; ++shift) {
            if (wordModel.get(shift).word === target[0].word) {
                if (shift > 0)
                    wordModel.remove(0, shift)
                return
            }
        }
    }

    function sync() {
        const target = wanted()
        dropLeading(target)
        for (let i = 0; i < target.length; ++i) {
            if (i >= wordModel.count) {
                wordModel.append({ word: target[i].word, live: target[i].live, revision: 0 })
                continue
            }
            const item = wordModel.get(i)
            if (item.word !== target[i].word)
                wordModel.set(i, { word: target[i].word, live: target[i].live, revision: item.revision + 1 })
            else if (item.live !== target[i].live)
                wordModel.setProperty(i, "live", target[i].live)
        }
        if (wordModel.count > target.length)
            wordModel.remove(target.length, wordModel.count - target.length)
    }

    onCommittedChanged: sync()
    onPartialChanged: sync()
    Component.onCompleted: sync()

    ListModel {
        id: wordModel
    }

    Text {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        visible: wordModel.count === 0 && transcript.placeholder !== ""
        text: transcript.placeholder
        wrapMode: Text.WordWrap
        horizontalAlignment: transcript.centered ? Text.AlignHCenter : Text.AlignLeft
        color: VoiceStyle.ink
        opacity: 0.4
        font.pixelSize: transcript.fontSize * 0.72
        font.weight: Font.Light
    }

    Flow {
        id: flow

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        spacing: transcript.fontSize * 0.28
        leftPadding: transcript.centered ? Math.max(0, (width - childrenRect.width) / 2) : 0

        Repeater {
            model: wordModel

            Text {
                id: wordText

                required property string word
                required property bool live
                required property int revision
                readonly property real restingOpacity: live ? 1 : 0.42

                text: word
                font.pixelSize: transcript.fontSize
                font.weight: live ? Font.Normal : Font.Light
                color: VoiceStyle.ink
                opacity: 0
                transform: Translate { id: rise }

                Component.onCompleted: appear.restart()
                onRevisionChanged: appear.restart()
                onRestingOpacityChanged: if (!appear.running) opacity = restingOpacity

                ParallelAnimation {
                    id: appear
                    NumberAnimation { target: wordText; property: "opacity"; from: 0; to: wordText.restingOpacity; duration: VoiceStyle.duration(260); easing.type: Easing.OutCubic }
                    NumberAnimation { target: rise; property: "y"; from: transcript.fontSize * 0.35; to: 0; duration: VoiceStyle.duration(320); easing.type: Easing.OutCubic }
                }

                Behavior on opacity {
                    enabled: !appear.running && VoiceStyle.animate
                    NumberAnimation { duration: VoiceStyle.duration(400); easing.type: Easing.OutCubic }
                }
            }
        }
    }
}
