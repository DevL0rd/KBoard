pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.devl0rd.kboard.settings

SettingRow {
    id: row

    readonly property var gifs: Modules.gif ? Modules.gif.api : null
    readonly property bool ready: gifs !== null && gifs.configured

    iconName: ready ? "checkmark" : "dialog-warning"
    description: {
        if (!gifs) {
            return "The GIF module isn't installed: " + Modules.errorFor("gif");
        }
        if (!gifs.configured) {
            return "GIFs need a KLIPY API key at build time. This copy of KBoard was built without one.";
        }
        return gifs.errorString.length > 0 ? gifs.errorString : "Connected. Searches go to KLIPY; nothing else leaves your computer.";
    }

    RowLayout {
        spacing: Kirigami.Units.largeSpacing

        QQC2.Label {
            text: row.gifs ? row.gifs.attribution : ""
            font.weight: Font.DemiBold
            opacity: 0.8
        }

        QQC2.Button {
            text: "klipy.com"
            icon.name: "internet-services"
            onClicked: Qt.openUrlExternally("https://klipy.com")
        }
    }
}
