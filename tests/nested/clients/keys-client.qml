import QtQuick
import QtQuick.Window

Window {
    id: root

    readonly property var args: Qt.application.arguments
    readonly property string label: args[args.length - 3]
    property string typed: ""

    width: Number(args[args.length - 2])
    height: Number(args[args.length - 1])
    visible: true
    title: root.label
    color: "#20252b"

    Item {
        anchors.fill: parent
        focus: true
        Keys.onPressed: event => {
            if (event.key === Qt.Key_Backspace)
                root.typed = root.typed.slice(0, -1)
            else if (event.text !== "")
                root.typed += event.text
            console.warn("kboard-test-keys:" + root.label + ":" + root.typed)
        }
    }

    Text {
        anchors.centerIn: parent
        color: "white"
        font.pixelSize: 28
        text: root.typed
    }
}
