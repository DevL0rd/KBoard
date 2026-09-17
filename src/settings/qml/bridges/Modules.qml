pragma Singleton
pragma ComponentBehavior: Bound

import QtQuick

QtObject {
    id: modules

    readonly property var bridges: ({ sound: "SoundBridge", voice: "VoiceBridge", gamepad: "GamepadBridge", emoji: "EmojiBridge", gif: "GifBridge", clipboard: "ClipboardBridge", typing: "TypingBridge", layouts: "LayoutsBridge" })
    property var errors: ({})
    property var sound: null
    property var voice: null
    property var gamepad: null
    property var emoji: null
    property var gif: null
    property var clipboard: null
    property var typing: null
    property var layouts: null

    function errorFor(key) {
        return errors[key] || "";
    }

    Component.onCompleted: {
        const failures = {};
        for (const key in bridges) {
            const component = Qt.createComponent("org.devl0rd.kboard.settings", bridges[key]);
            if (component.status === Component.Ready) {
                modules[key] = component.createObject(modules);
            } else {
                failures[key] = component.errorString().trim();
                console.warn("KBoard settings:", bridges[key], "is unavailable:", failures[key]);
            }
        }
        errors = failures;
    }
}
