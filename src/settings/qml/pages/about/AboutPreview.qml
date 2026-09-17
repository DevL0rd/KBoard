pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.devl0rd.kboard.config
import org.devl0rd.kboard.settings

RowLayout {
    id: preview

    readonly property color accent: Settings.accentMode === 1 ? Settings.accentColor : Kirigami.Theme.highlightColor

    spacing: Kirigami.Units.gridUnit * 2

    TypingLoop {
        id: typing
        keys: ["k", "b", "o", "a", "r", "d"]
        keyMs: 300
        restMs: 1600
    }

    Item {
        Layout.fillWidth: true
    }

    Item {
        Layout.fillHeight: true
        Layout.preferredWidth: height

        Rectangle {
            anchors.centerIn: parent
            width: parent.height * (0.8 + 0.05 * typing.press)
            height: width
            radius: width / 2
            color: Qt.alpha(preview.accent, 0.12)
            border.color: Qt.alpha(preview.accent, 0.4)
        }

        Kirigami.Icon {
            anchors.centerIn: parent
            width: parent.height * 0.5
            height: width
            source: "input-keyboard-virtual"
        }
    }

    ColumnLayout {
        spacing: Kirigami.Units.smallSpacing
        Layout.alignment: Qt.AlignVCenter

        Kirigami.Heading {
            level: 1
            text: SystemInfo.displayName
        }

        QQC2.Label {
            text: "Version " + SystemInfo.version
            opacity: 0.75
        }

        MiniKeyboard {
            Layout.preferredWidth: Kirigami.Units.gridUnit * 14
            Layout.preferredHeight: Kirigami.Units.gridUnit * 5
            showHints: false
            pressedKey: typing.pressedKey
            press: typing.press
            popupKey: Settings.keyPopup ? typing.pressedKey : ""
            popup: typing.popup
        }
    }

    Item {
        Layout.fillWidth: true
    }
}
