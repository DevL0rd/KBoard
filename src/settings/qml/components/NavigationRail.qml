pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.devl0rd.kboard.settings

ColumnLayout {
    id: rail

    signal importRequested
    signal exportRequested

    readonly property bool searching: search.text.trim().length > 0
    readonly property var results: searching ? SettingsCatalog.search(search.text) : []

    function focusSearch() {
        search.forceActiveFocus();
        search.selectAll();
    }

    function clearSearch() {
        AppNavigation.searchText = "";
    }

    function step(delta) {
        const pages = SettingsCatalog.pages;
        const index = pages.findIndex(page => page.id === AppNavigation.page);
        const next = (index + delta + pages.length) % pages.length;
        AppNavigation.open(pages[next].id);
    }

    function choose(entry) {
        AppNavigation.open(entry.page, entry.label);
        AppNavigation.searchText = "";
        pageList.forceActiveFocus();
    }

    spacing: 0

    ColumnLayout {
        Layout.fillWidth: true
        Layout.margins: Kirigami.Units.largeSpacing
        Layout.bottomMargin: Kirigami.Units.smallSpacing
        spacing: 0

        RowLayout {
            spacing: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            Layout.bottomMargin: Kirigami.Units.largeSpacing

            Kirigami.Icon {
                source: "input-keyboard-virtual"
                Layout.preferredWidth: Kirigami.Units.iconSizes.medium
                Layout.preferredHeight: Kirigami.Units.iconSizes.medium
            }

            ColumnLayout {
                spacing: 0
                Layout.fillWidth: true

                Kirigami.Heading {
                    level: 3
                    text: "Settings"
                    Layout.fillWidth: true
                }

                QQC2.Label {
                    text: "KBoard"
                    opacity: 0.7
                    font: Kirigami.Theme.smallFont
                }
            }
        }

        Kirigami.SearchField {
            id: search
            Layout.fillWidth: true
            placeholderText: "Search settings…"
            text: AppNavigation.searchText
            onTextEdited: AppNavigation.searchText = text
            KeyNavigation.down: rail.searching ? resultList : pageList
            Keys.onReturnPressed: {
                if (rail.results.length > 0) {
                    rail.choose(rail.results[Math.max(0, resultList.currentIndex)]);
                }
            }
            Keys.onEnterPressed: event => Keys.returnPressed(event)
            Keys.onEscapePressed: event => {
                if (text.length > 0) {
                    rail.clearSearch();
                } else {
                    event.accepted = false;
                }
            }
            onTextChanged: resultList.currentIndex = 0
        }
    }

    ListView {
        id: pageList
        Layout.fillWidth: true
        Layout.fillHeight: true
        visible: !rail.searching
        clip: true
        model: SettingsCatalog.pages
        currentIndex: SettingsCatalog.pages.findIndex(page => page.id === AppNavigation.page)
        keyNavigationEnabled: true
        activeFocusOnTab: true
        spacing: 2
        leftMargin: Kirigami.Units.smallSpacing * 2
        rightMargin: Kirigami.Units.smallSpacing * 2
        highlightMoveDuration: Motion.short
        Keys.onReturnPressed: AppNavigation.open(SettingsCatalog.pages[currentIndex].id)
        Keys.onSpacePressed: AppNavigation.open(SettingsCatalog.pages[currentIndex].id)
        onCurrentIndexChanged: {
            if (activeFocus && currentIndex >= 0) {
                AppNavigation.open(SettingsCatalog.pages[currentIndex].id);
            }
        }

        delegate: RailItem {
            required property var modelData
            required property int index
            width: ListView.view.width - ListView.view.leftMargin - ListView.view.rightMargin
            title: modelData.title
            iconName: modelData.icon
            selected: modelData.id === AppNavigation.page
            keyboardFocus: ListView.view.activeFocus && ListView.isCurrentItem
            onClicked: AppNavigation.open(modelData.id)
        }
    }

    ListView {
        id: resultList
        Layout.fillWidth: true
        Layout.fillHeight: true
        visible: rail.searching
        clip: true
        model: rail.results
        keyNavigationEnabled: true
        spacing: 2
        leftMargin: Kirigami.Units.smallSpacing * 2
        rightMargin: Kirigami.Units.smallSpacing * 2
        Keys.onReturnPressed: rail.choose(rail.results[currentIndex])
        Keys.onUpPressed: event => {
            if (currentIndex === 0) {
                search.forceActiveFocus();
            } else {
                event.accepted = false;
            }
        }

        delegate: RailItem {
            required property var modelData
            required property int index
            width: ListView.view.width - ListView.view.leftMargin - ListView.view.rightMargin
            title: modelData.label
            subtitle: modelData.pageTitle + (modelData.section ? " · " + modelData.section : "")
            iconName: modelData.icon
            query: search.text
            selected: false
            keyboardFocus: ListView.isCurrentItem && (ListView.view.activeFocus || search.activeFocus)
            onClicked: rail.choose(modelData)
        }

        Kirigami.PlaceholderMessage {
            anchors.centerIn: parent
            width: parent.width - Kirigami.Units.gridUnit * 2
            visible: rail.searching && rail.results.length === 0
            icon.name: "edit-find"
            text: "No settings match"
            explanation: "Try a different word, like \"sound\" or \"split\"."
        }
    }

    Kirigami.Separator {
        Layout.fillWidth: true
    }

    ColumnLayout {
        Layout.fillWidth: true
        Layout.margins: Kirigami.Units.smallSpacing * 2
        spacing: 0

        RailItem {
            Layout.fillWidth: true
            iconName: "document-import"
            title: "Import settings…"
            focusPolicy: Qt.StrongFocus
            onClicked: rail.importRequested()
        }

        RailItem {
            Layout.fillWidth: true
            iconName: "document-export"
            title: "Export settings…"
            focusPolicy: Qt.StrongFocus
            onClicked: rail.exportRequested()
        }
    }
}
