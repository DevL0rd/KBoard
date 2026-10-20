pragma ComponentBehavior: Bound

import QtQuick
import org.devl0rd.kboard.ui

Item {
    id: live

    readonly property alias view: keyboardView

    function demoPress(label) {
        keyboardView.demoPress(label);
    }

    function demoGlide(word) {
        keyboardView.demoGlide(word);
    }

    KeyboardBackdrop {
        anchors.fill: parent
    }

    KeyboardView {
        id: keyboardView
        anchors.fill: parent
        preview: true
    }
}
