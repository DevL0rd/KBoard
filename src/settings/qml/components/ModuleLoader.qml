pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

Item {
    id: holder

    property string typeName
    property string module
    property var properties: ({})
    readonly property alias item: loader.item
    readonly property string errorString: component.status === Component.Error ? component.errorString().trim() : ""
    readonly property var component: Qt.createComponent("org.devl0rd.kboard.settings", typeName, Component.PreferSynchronous)

    Loader {
        id: loader
        anchors.fill: parent
        active: holder.component.status === Component.Ready
        sourceComponent: holder.component
        onLoaded: {
            for (const key in holder.properties) {
                item[key] = holder.properties[key];
            }
        }
    }

    ColumnLayout {
        anchors.centerIn: parent
        width: Math.min(parent.width, Kirigami.Units.gridUnit * 24)
        visible: holder.errorString.length > 0
        spacing: Kirigami.Units.smallSpacing

        Kirigami.Icon {
            source: "dialog-error"
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: Kirigami.Units.iconSizes.medium
            Layout.preferredHeight: Kirigami.Units.iconSizes.medium
        }

        QQC2.Label {
            text: "The KBoard " + holder.module + " module isn't available"
            font.weight: Font.DemiBold
            horizontalAlignment: Text.AlignHCenter
            Layout.fillWidth: true
        }

        QQC2.Label {
            text: holder.errorString
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
            opacity: 0.7
            font: Kirigami.Theme.smallFont
            maximumLineCount: 3
            elide: Text.ElideRight
            Layout.fillWidth: true
        }
    }
}
