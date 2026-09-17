pragma ComponentBehavior: Bound

import QtQuick
import org.devl0rd.kboard.settings

Item {
    id: nav

    property var window
    readonly property var gamepad: Modules.gamepad ? Modules.gamepad.api : null

    signal pageStep(int delta)
    signal backRequested

    Connections {
        target: nav.gamepad
        enabled: nav.gamepad !== null && nav.window && nav.window.active
        ignoreUnknownSignals: true

        function onNavigate(dx, dy) {
            const item = nav.window.contentItem;
            if (dy !== 0) {
                FocusDriver.moveFocus(item, dy > 0);
            } else if (dx !== 0) {
                FocusDriver.sendKey(item, dx > 0 ? Qt.Key_Right : Qt.Key_Left);
            }
        }

        function onButtonPressed(button) {
            const item = nav.window.contentItem;
            switch (button) {
            case "a":
                FocusDriver.sendKey(item, Qt.Key_Space);
                break;
            case "b":
                FocusDriver.sendKey(item, Qt.Key_Escape);
                nav.backRequested();
                break;
            case "leftshoulder":
                nav.pageStep(-1);
                break;
            case "rightshoulder":
                nav.pageStep(1);
                break;
            }
        }
    }
}
