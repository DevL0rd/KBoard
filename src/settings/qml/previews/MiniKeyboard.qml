pragma ComponentBehavior: Bound

import QtQuick
import org.kde.kirigami as Kirigami
import org.devl0rd.kboard.config
import "KeyboardModel.js" as KeyboardModel

Item {
    id: kb

    property int keyStyle: Settings.keyStyle
    property real keyRadius: Settings.keyRadius
    property real keyGap: Settings.keyGap
    property real labelScale: Settings.labelScale
    property real backgroundOpacity: Settings.backgroundOpacity
    property bool blur: Settings.blurEnabled
    property bool showHints: Settings.showKeyHints
    property bool numberRow: Settings.showNumberRow
    property bool desktopRow: Settings.showDesktopRow
    property real split: 0
    property real splitHalfWidth: Settings.splitHalfWidth
    property color accent: Settings.accentMode === 1 ? Settings.accentColor : Kirigami.Theme.highlightColor
    property real metric: Math.max(0.2, width / 560)
    property string spaceLabel: ""
    property Item backdrop: null
    property bool shifted: false

    property string pressedKey
    property real press: 0
    property string popupKey
    property real popup: 0
    property string rippleKey
    property real ripple: 0
    property point rippleOrigin: Qt.point(0.5, 0.5)
    property string holdKey
    property real hold: 0
    property string accentKey
    property real accentOpen: 0
    property int accentIndex: -1
    property string focusKey
    property real focusGlow: 1
    property var litKeys: []
    property var trail: []
    property real trailOpacity: 1

    readonly property point backdropOrigin: {
        x;
        y;
        width;
        height;
        return backdrop ? backdrop.mapFromItem(kb, 0, 0) : Qt.point(0, 0);
    }
    readonly property var model: KeyboardModel.layout({ numberRow: numberRow, desktopRow: desktopRow, spaceLabel: spaceLabel })
    readonly property real unitW: width / 10
    readonly property real unitH: height / model.height
    readonly property real gapPx: keyGap * metric
    readonly property real radiusPx: keyRadius * metric
    readonly property real splitFactor: 1 + (2 * splitHalfWidth - 1) * split
    readonly property real leftEdge: 5 * splitFactor * unitW
    readonly property real rightEdge: width - leftEdge
    readonly property color panelColor: Kirigami.Theme.backgroundColor
    readonly property color keyColor: buttonColors.Kirigami.Theme.backgroundColor
    readonly property color textColor: Kirigami.Theme.textColor
    readonly property bool darkTheme: Kirigami.ColorUtils.brightnessForColor(Kirigami.Theme.backgroundColor) === Kirigami.ColorUtils.Dark

    function aspectHeight(forWidth) {
        return forWidth / 10 * model.height * 0.72;
    }

    function keyData(id) {
        const keys = model.keys;
        for (let i = 0; i < keys.length; ++i) {
            if (keys[i].id === id && keys[i].part !== "left" && keys[i].part !== "right") {
                return keys[i];
            }
        }
        return null;
    }

    function keyRect(id) {
        const key = keyData(id);
        if (!key) {
            return Qt.rect(0, 0, 0, 0);
        }
        const s = KeyboardModel.splitX(key.x, key.w, split, splitHalfWidth);
        return Qt.rect(s.x * unitW, key.y * unitH, s.w * unitW, key.h * unitH);
    }

    function keyCenter(id) {
        const r = keyRect(id);
        return Qt.point(r.x + r.width / 2, r.y + r.height / 2);
    }

    readonly property var keyPalette: ({ key: keyColor, panel: panelColor, accent: accent, text: textColor, dark: darkTheme })

    Item {
        id: buttonColors
        Kirigami.Theme.colorSet: Kirigami.Theme.Button
        Kirigami.Theme.inherit: false
    }

    KeyboardPanel {
        kb: kb
    }

    KeyCaps {
        kb: kb
    }

    GlideTrail {
        kb: kb
        z: 5
    }

    KeyBubble {
        kb: kb
    }

    AccentStrip {
        kb: kb
    }
}
