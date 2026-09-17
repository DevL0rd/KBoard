pragma ComponentBehavior: Bound

import QtQuick
import org.kde.kirigami as Kirigami

SettingRow {
    id: row

    property var options: []
    property Component preview: null
    property real cardHeight: Kirigami.Units.gridUnit * 6
    property real minimumCardWidth: Kirigami.Units.gridUnit * 7

    wideControl: true

    ChoiceCards {
        width: parent.width
        options: row.options
        preview: row.preview
        cardHeight: row.cardHeight
        minimumCardWidth: row.minimumCardWidth
        currentValue: row.current
        onChosen: value => row.apply(value)
    }
}
