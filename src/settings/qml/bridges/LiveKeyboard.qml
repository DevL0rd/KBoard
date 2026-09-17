pragma ComponentBehavior: Bound

import QtQuick
import org.devl0rd.kboard.ui

Item {
    id: live

    readonly property alias view: view

    function demoPress(label) {
        view.demoPress(label);
    }

    function demoGlide(word) {
        view.demoGlide(word);
    }

    KeyboardBackdrop {
        anchors.fill: parent
    }

    KeyboardView {
        id: view
        anchors.fill: parent
        preview: true
    }
}
