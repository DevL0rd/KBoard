pragma ComponentBehavior: Bound

import QtQuick

SettingRow {
    id: row

    property var options: []

    Segmented {
        options: row.options
        currentValue: row.current
        onChosen: value => row.apply(value)
    }
}
