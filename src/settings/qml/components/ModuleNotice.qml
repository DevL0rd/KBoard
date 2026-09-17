pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.InlineMessage {
    property string module
    property string what

    Layout.fillWidth: true
    Layout.maximumWidth: Kirigami.Units.gridUnit * 46
    Layout.alignment: Qt.AlignHCenter
    Layout.leftMargin: Kirigami.Units.largeSpacing
    Layout.rightMargin: Kirigami.Units.largeSpacing
    type: Kirigami.MessageType.Error
    visible: Modules.errorFor(module).length > 0
    text: what + " isn't available because the KBoard " + module + " module failed to load: " + Modules.errorFor(module)
}
