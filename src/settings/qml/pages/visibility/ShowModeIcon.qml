pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

RowLayout {
    property var choice: null
    readonly property var icons: choice === null ? [] : [["dialog-cancel"], ["input-touchscreen"], ["input-touchscreen", "input-mouse"]][choice.value]

    spacing: Kirigami.Units.smallSpacing

    Item {
        Layout.fillWidth: true
    }

    Repeater {
        model: parent.icons

        Kirigami.Icon {
            required property string modelData
            source: modelData
            Layout.preferredWidth: Kirigami.Units.iconSizes.smallMedium
            Layout.preferredHeight: Kirigami.Units.iconSizes.smallMedium
        }
    }

    Item {
        Layout.fillWidth: true
    }
}
