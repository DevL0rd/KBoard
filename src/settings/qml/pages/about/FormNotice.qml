pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.devl0rd.kboard.settings

SettingRow {
    id: row

    property var notice: ({})

    label: notice.name || ""
    description: (notice.use || "") + " · " + (notice.licence || "")

    QQC2.ToolButton {
        icon.name: "internet-services"
        text: "Open website for " + row.notice.name
        display: QQC2.AbstractButton.IconOnly
        onClicked: Qt.openUrlExternally(row.notice.url)
        QQC2.ToolTip.text: row.notice.url || ""
        QQC2.ToolTip.visible: hovered
        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
    }
}
