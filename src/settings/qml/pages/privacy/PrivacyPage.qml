pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.devl0rd.kboard.config
import org.devl0rd.kboard.settings

SettingsPage {
    id: page

    readonly property var engine: Modules.typing ? Modules.typing.api : null
    readonly property var history: Modules.clipboard ? Modules.clipboard.api : null
    readonly property var voice: Modules.voice ? Modules.voice.api : null
    readonly property int modelMegabytes: voice ? voice.models.filter(model => model.downloaded).reduce((sum, model) => sum + model.sizeMb, 0) : 0

    preview: DataFlowPreview {}

    ModuleNotice {
        module: "typing"
        what: "Learned words"
    }

    Kirigami.PromptDialog {
        id: forgetAll
        title: "Forget all learned words?"
        subtitle: "Suggestions and autocorrect go back to the built-in dictionary. This can't be undone."
        standardButtons: Kirigami.Dialog.Cancel
        customFooterActions: [
            Kirigami.Action {
                text: "Forget all"
                icon.name: "edit-clear-all"
                onTriggered: {
                    page.engine.clearLearned();
                    forgetAll.close();
                }
            }
        ]
    }

    Kirigami.PromptDialog {
        id: clearHistory
        title: "Clear clipboard history?"
        subtitle: "Pinned items are removed too. This can't be undone."
        standardButtons: Kirigami.Dialog.Cancel
        customFooterActions: [
            Kirigami.Action {
                text: "Clear history"
                icon.name: "edit-clear-history"
                onTriggered: {
                    page.history.clear(true);
                    clearHistory.close();
                }
            }
        ]
    }

    Section {
        title: "Learning"

        SwitchRow {
            label: "Learn new words"
            description: "Remember words and phrases you type to improve suggestions. Never learns in password fields."
            iconName: "tools-check-spelling"
            setting: "learnWords"
        }

        LearnedWordsRow {
            label: "Learned words"
            description: "Remove a word to stop it being suggested"
            iconName: "view-list-text"
            onClearRequested: forgetAll.open()
        }
    }

    Section {
        title: "Clipboard"

        SettingRow {
            label: "Clipboard history"
            description: page.history ? page.history.count + " items stored" + (Settings.clipboardPersist ? " on disk" : " in memory") : "Clipboard history isn't available"
            iconName: "edit-paste"

            RowLayout {
                spacing: Kirigami.Units.smallSpacing

                QQC2.Button {
                    text: "Settings"
                    icon.name: "configure"
                    onClicked: AppNavigation.open("clipboard", "Keep items for")
                }

                QQC2.Button {
                    text: "Clear…"
                    icon.name: "edit-clear-history"
                    enabled: page.history !== null && page.history.count > 0
                    onClicked: clearHistory.open()
                }
            }
        }
    }

    Section {
        title: "Voice"

        SettingRow {
            label: "Voice data"
            description: "Audio is processed on this computer and thrown away right after. Downloaded models use " + (page.modelMegabytes >= 1024 ? (page.modelMegabytes / 1024).toFixed(1) + " GB" : page.modelMegabytes + " MB") + "."
            iconName: "audio-input-microphone"

            QQC2.Button {
                text: "Open models folder"
                icon.name: "folder-open"
                onClicked: Qt.openUrlExternally("file://" + KBoardPaths.userDataDir + "/models")
            }
        }
    }
}
