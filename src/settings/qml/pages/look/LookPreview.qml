pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Effects
import org.kde.kirigami as Kirigami
import org.devl0rd.kboard.config
import org.devl0rd.kboard.settings

RowLayout {
    id: preview

    readonly property string caption: "Height " + Math.round(Settings.heightLandscape * 100) + "% in landscape, " + Math.round(Settings.heightPortrait * 100) + "% in portrait"
    readonly property color accent: Settings.accentMode === 1 ? Settings.accentColor : Kirigami.Theme.highlightColor

    function pressLive(keyboard: var, label: string) {
        keyboard.demoPress(label);
    }

    spacing: Kirigami.Units.gridUnit

    TypingLoop {
        id: typing
        keys: ["h", "e", "l", "l", "o", "space", "k", "b", "o", "a", "r", "d"]
    }

    component DockedScreen: ScreenFrame {
        id: screen
        property real share
        Layout.fillHeight: true
        Layout.preferredWidth: height * aspect
        accent: preview.accent
        window.text: typing.typed
        window.focused: true
        window.height: screenHeight * (1 - share) - screenHeight * 0.12

        MiniKeyboard {
            anchors.bottom: parent.bottom
            width: parent.width
            height: parent.height * screen.share
            metric: Math.max(0.25, parent.width / 560) * (screen.aspect < 1 ? 1.6 : 1)
            backdrop: screen.wallpaper
            showHints: false
            pressedKey: typing.pressedKey
            press: typing.press
            rippleKey: Settings.ripple ? typing.pressedKey : ""
            ripple: typing.ripple
        }
    }

    DockedScreen {
        aspect: 16 / 10
        share: Settings.heightLandscape
    }

    Item {
        Layout.fillWidth: true
        Layout.fillHeight: true

        Wallpaper {
            id: liveWallpaper
            anchors.fill: parent
            radius: Kirigami.Units.cornerRadius * 2
            accent: preview.accent
        }

        Item {
            anchors.fill: live
            clip: true
            visible: Settings.blurEnabled

            MultiEffect {
                x: -live.x
                y: -live.y
                width: liveWallpaper.width
                height: liveWallpaper.height
                source: liveWallpaper
                blurEnabled: true
                blur: 1
                blurMax: 48
                saturation: 0.15
            }
        }

        ModuleLoader {
            id: live
            anchors.fill: parent
            anchors.topMargin: parent.height * 0.12
            typeName: "LiveKeyboard"
            module: "ui"
        }

        Connections {
            target: typing
            function onPressedKeyChanged() {
                if (typing.pressedKey.length > 0 && live.item) {
                    preview.pressLive(live.item, typing.pressedKey);
                }
            }
        }

        Rectangle {
            anchors.fill: parent
            radius: Kirigami.Units.cornerRadius * 2
            color: "transparent"
            border.color: Qt.alpha(Kirigami.Theme.textColor, 0.2)
        }
    }

    DockedScreen {
        aspect: 10 / 16
        share: Settings.heightPortrait
    }
}
