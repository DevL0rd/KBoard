import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

Kirigami.FormLayout {
    property string cfg_tapAction
    property string cfg_middleClickPanel
    property alias cfg_showVisibleIndicator: showVisibleIndicator.checked

    QQC2.ComboBox {
        Kirigami.FormData.label: i18n("Clicking the panel icon:")
        textRole: "text"
        valueRole: "value"
        model: [
            { text: i18n("Shows or hides the keyboard"), value: "toggle" },
            { text: i18n("Opens the quick actions"), value: "popup" }
        ]
        Component.onCompleted: currentIndex = Math.max(0, indexOfValue(cfg_tapAction))
        onActivated: cfg_tapAction = currentValue
    }

    QQC2.ComboBox {
        Kirigami.FormData.label: i18n("Middle-click opens:")
        textRole: "text"
        valueRole: "value"
        model: [
            { text: i18n("Voice typing"), value: "voice" },
            { text: i18n("Emoji"), value: "emoji" },
            { text: i18n("GIFs"), value: "gif" },
            { text: i18n("Clipboard"), value: "clipboard" },
            { text: i18n("Text editing"), value: "edit" },
            { text: i18n("Nothing"), value: "" }
        ]
        Component.onCompleted: currentIndex = Math.max(0, indexOfValue(cfg_middleClickPanel))
        onActivated: cfg_middleClickPanel = currentValue
    }

    QQC2.CheckBox {
        id: showVisibleIndicator
        text: i18n("Underline the icon while the keyboard is showing")
    }

    QQC2.Label {
        text: i18n("Press and hold the panel icon to open the quick actions.")
        opacity: 0.7
    }
}
