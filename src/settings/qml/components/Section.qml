pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

ColumnLayout {
    id: section

    property string title
    property string summary
    default property alias rows: card.delegates

    spacing: 0
    Layout.fillWidth: true

    FormCard.FormHeader {
        title: section.title
        maximumWidth: Kirigami.Units.gridUnit * 46
        Layout.fillWidth: true
    }

    FormCard.FormCard {
        id: card
        maximumWidth: Kirigami.Units.gridUnit * 46
        Layout.fillWidth: true
    }
}
