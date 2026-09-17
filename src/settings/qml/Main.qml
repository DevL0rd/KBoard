pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Dialogs
import QtCore
import org.kde.kirigami as Kirigami
import org.devl0rd.kboard.settings

Kirigami.ApplicationWindow {
    id: window

    readonly property var currentPage: SettingsCatalog.page(AppNavigation.page)

    title: currentPage.title ? currentPage.title + " — KBoard Settings" : "KBoard Settings"
    width: Kirigami.Units.gridUnit * 64
    height: Kirigami.Units.gridUnit * 46
    minimumWidth: Kirigami.Units.gridUnit * 36
    minimumHeight: Kirigami.Units.gridUnit * 28
    pageStack.visible: false

    Shortcut {
        sequences: [StandardKey.Find]
        onActivated: rail.focusSearch()
    }

    Shortcut {
        sequences: [StandardKey.Undo]
        enabled: SettingsStore.canUndo
        onActivated: SettingsStore.undo()
    }

    Shortcut {
        sequences: ["Ctrl+PgDown"]
        onActivated: rail.step(1)
    }

    Shortcut {
        sequences: ["Ctrl+PgUp"]
        onActivated: rail.step(-1)
    }

    ControllerNavigation {
        window: window
        onPageStep: delta => rail.step(delta)
        onBackRequested: rail.clearSearch()
    }

    FileDialog {
        id: exportDialog
        title: "Export KBoard settings"
        fileMode: FileDialog.SaveFile
        defaultSuffix: "kboardsettings"
        currentFolder: StandardPaths.writableLocation(StandardPaths.DocumentsLocation)
        selectedFile: currentFolder + "/kboard.kboardsettings"
        nameFilters: ["KBoard settings (*.kboardsettings)", "All files (*)"]
        onAccepted: SettingsStore.exportTo(selectedFile)
    }

    FileDialog {
        id: importDialog
        title: "Import KBoard settings"
        fileMode: FileDialog.OpenFile
        currentFolder: StandardPaths.writableLocation(StandardPaths.DocumentsLocation)
        nameFilters: ["KBoard settings (*.kboardsettings kboardrc)", "All files (*)"]
        onAccepted: SettingsStore.importFrom(selectedFile)
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        NavigationRail {
            id: rail
            Layout.fillHeight: true
            Layout.preferredWidth: Kirigami.Units.gridUnit * 14
            Layout.fillWidth: false
            onImportRequested: importDialog.open()
            onExportRequested: exportDialog.open()
        }

        Kirigami.Separator {
            Layout.fillHeight: true
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            PageHeader {
                Layout.fillWidth: true
                page: window.currentPage
            }

            StatusMessages {
                Layout.fillWidth: true
                Layout.fillHeight: false
            }

            PageHost {
                Layout.fillWidth: true
                Layout.fillHeight: true
                page: window.currentPage
            }
        }
    }
}
