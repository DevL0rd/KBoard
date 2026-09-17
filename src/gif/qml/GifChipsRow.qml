pragma ComponentBehavior: Bound

import QtQuick
import org.kde.kirigami as Kirigami
import org.devl0rd.kboard.gif

ListView {
    id: chips

    property string selectedSection
    property string categoryQuery
    property string searchText
    property real animationScale: 1

    signal sectionSelected(string name, string query)
    signal suggestionSelected(string text)

    readonly property bool typing: searchText.trim() !== ""
    readonly property bool suggesting: typing && GifStore.suggestions.length > 0

    orientation: ListView.Horizontal
    spacing: Kirigami.Units.smallSpacing
    leftMargin: Kirigami.Units.largeSpacing
    rightMargin: Kirigami.Units.largeSpacing
    clip: true
    boundsBehavior: Flickable.StopAtBounds
    highlightMoveDuration: Kirigami.Units.longDuration * animationScale
    highlightResizeDuration: Kirigami.Units.longDuration * animationScale
    preferredHighlightBegin: Kirigami.Units.gridUnit * 3
    preferredHighlightEnd: width - Kirigami.Units.gridUnit * 3
    highlightRangeMode: ListView.ApplyRange

    model: suggesting
        ? GifStore.suggestions.map(s => ({ key: "suggestion", name: s, query: s, icon: "search" }))
        : [
            { key: "recents", name: qsTr("Recent"), query: "", icon: "document-open-recent" },
            { key: "favorites", name: qsTr("Favorites"), query: "", icon: "emblem-favorite" },
            { key: "trending", name: qsTr("Trending"), query: "", icon: "view-statistics" }
        ].concat(GifStore.categories.map(c => ({ key: "category", name: c.name, query: c.query, icon: "" })))

    currentIndex: suggesting || typing ? -1 : model.findIndex(entry => entry.key === selectedSection && (selectedSection !== "category" || entry.query === categoryQuery))

    highlight: Rectangle {
        radius: height / 2
        color: Kirigami.Theme.highlightColor
    }

    delegate: GifChip {
        required property var modelData
        text: modelData.key === "category" ? modelData.name.charAt(0).toUpperCase() + modelData.name.slice(1) : modelData.name
        iconName: modelData.icon
        selected: ListView.isCurrentItem
        animationScale: chips.animationScale
        onClicked: modelData.key === "suggestion" ? chips.suggestionSelected(modelData.query) : chips.sectionSelected(modelData.key, modelData.query)
    }

    add: Transition {
        NumberAnimation { property: "opacity"; from: 0; to: 1; duration: Kirigami.Units.longDuration * chips.animationScale; easing.type: Easing.OutCubic }
    }
}
