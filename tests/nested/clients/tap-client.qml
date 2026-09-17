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
    color: "#3a4a5a"

    TapHandler {
        onTapped: point => console.warn("kboard-test-tap:" + root.label + ":" + Math.round(point.position.x) + "," + Math.round(point.position.y))
    }
}
