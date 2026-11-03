import QtQuick
import QtQuick.Window

Window {
    id: root

    readonly property var args: Qt.application.arguments
    readonly property string label: args[args.length - 3]

    width: Number(args[args.length - 2])
    height: Number(args[args.length - 1])
    visible: true
    title: root.label
    color: "#20252b"

    function report(phase, event) {
        const mods = (event.modifiers & Qt.ControlModifier ? "C" : "") + (event.modifiers & Qt.ShiftModifier ? "S" : "")
            + (event.modifiers & Qt.AltModifier ? "A" : "") + (event.modifiers & Qt.MetaModifier ? "M" : "")
        console.warn("kboard-test-mod:" + phase + ":" + event.key + ":" + mods + ":" + event.text.charCodeAt(0))
    }

    TextInput {
        anchors.fill: parent
        focus: true
        onTextChanged: console.warn("kboard-test-text:" + text)
        color: "white"
        font.pixelSize: 28
        Keys.onPressed: event => root.report("down", event)
        Keys.onReleased: event => root.report("up", event)
    }
}
