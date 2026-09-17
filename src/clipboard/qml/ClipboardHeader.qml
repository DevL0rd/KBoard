pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.devl0rd.kboard.clipboard

RowLayout {
    id: header

    property string filter: "all"
    property bool clearEnabled: true
    readonly property alias clearArmed: clearButton.armed

    signal backRequested()
    signal filterRequested(string key)
    signal clearConfirmed()

    function requestClear() {
        clearButton.tap()
    }

    spacing: Kirigami.Units.smallSpacing

    HeaderButton {
        iconName: "go-previous"
        tooltip: i18n("Back to keyboard")
        onClicked: header.backRequested()
    }

    ColumnLayout {
        spacing: 0
        Layout.rightMargin: Kirigami.Units.largeSpacing
        Layout.minimumWidth: Kirigami.Units.gridUnit * 6

        Text {
            text: i18n("Clipboard")
            color: Kirigami.Theme.textColor
            font.pointSize: Kirigami.Theme.defaultFont.pointSize * 1.25
            font.weight: Font.DemiBold
        }

        Text {
            readonly property string items: i18np("%1 item", "%1 items", ClipboardHistory.count)
            text: ClipboardHistory.paused ? i18n("Paused")
                : ClipboardHistory.pinnedCount > 0 ? i18nc("item count, pinned count", "%1 · %2", items, i18np("%1 pinned", "%1 pinned", ClipboardHistory.pinnedCount))
                : items
            color: ClipboardHistory.paused ? Kirigami.Theme.neutralTextColor : Kirigami.Theme.disabledTextColor
            font: Kirigami.Theme.smallFont
        }
    }

    Flickable {
        Layout.fillWidth: true
        Layout.preferredHeight: chips.implicitHeight
        contentWidth: chips.implicitWidth
        flickableDirection: Flickable.HorizontalFlick
        boundsBehavior: Flickable.StopAtBounds
        interactive: contentWidth > width
        clip: true

        Row {
            id: chips
            spacing: Kirigami.Units.smallSpacing

            Repeater {
                model: [
                    { key: "all", label: i18n("All"), icon: "view-list-icons" },
                    { key: "pinned", label: i18n("Pinned"), icon: "pin" },
                    { key: "images", label: i18n("Images"), icon: "image-x-generic" },
                    { key: "links", label: i18n("Links"), icon: "link" }
                ]

                delegate: FilterChip {
                    required property var modelData
                    label: modelData.label
                    iconName: modelData.icon
                    checked: header.filter === modelData.key
                    onClicked: header.filterRequested(modelData.key)
                }
            }
        }
    }

    HeaderButton {
        iconName: ClipboardHistory.paused ? "media-playback-start" : "media-playback-pause"
        tooltip: ClipboardHistory.paused ? i18n("Resume saving copies") : i18n("Pause saving copies")
        highlighted: ClipboardHistory.paused
        enabled: ClipboardHistory.enabled
        onClicked: ClipboardHistory.paused = !ClipboardHistory.paused
    }

    ClearButton {
        id: clearButton
        enabled: header.clearEnabled
        confirmText: ClipboardHistory.pinnedCount > 0 ? i18n("Tap again to clear (keeps pinned)") : i18n("Tap again to clear all")
        onConfirmed: header.clearConfirmed()
    }
}
