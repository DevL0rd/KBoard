pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

FocusScope {
    id: page

    property string pageId
    property Component preview: null
    property real previewHeight: Kirigami.Units.gridUnit * 13
    default property alias content: column.data
    readonly property Flickable flickable: scroll.contentItem as Flickable
    readonly property alias previewItem: stage.item

    PreviewStage {
        id: stage
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        implicitHeight: page.previewHeight
        visible: page.preview !== null
        height: visible ? implicitHeight : 0
        sourceComponent: page.preview
    }

    QQC2.ScrollView {
        id: scroll
        anchors.top: stage.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        QQC2.ScrollBar.horizontal.policy: QQC2.ScrollBar.AlwaysOff
        contentWidth: availableWidth
        bottomPadding: Kirigami.Units.gridUnit

        ColumnLayout {
            id: column
            width: scroll.availableWidth
            spacing: 0

            Item {
                Layout.preferredHeight: Kirigami.Units.gridUnit
            }
        }
    }
}
