import QtQuick
import org.kde.kirigami as Kirigami
import org.devl0rd.kboard.config
import org.devl0rd.kboard.gif

Item {
    id: panel

    property string searchText: ""
    property bool searchActive: false
    property string section: GifStore.recents.count > 0 ? "recents" : "trending"
    property string categoryQuery: ""
    property int columns: Math.max(2, Math.min(4, Math.floor(width / (Kirigami.Units.gridUnit * 9))))

    readonly property real animationScale: Settings.animationsEnabled ? 1 / Math.max(0.25, Settings.animationSpeed) : 0
    readonly property string effectiveQuery: searchText.trim() !== "" ? searchText : (section === "category" ? categoryQuery : "")
    readonly property bool showingSearch: effectiveQuery.trim() !== ""
    readonly property GifModel currentModel: showingSearch ? GifStore.results
        : section === "recents" ? GifStore.recents
        : section === "favorites" ? GifStore.favorites
        : GifStore.trending
    readonly property bool remoteFeed: showingSearch || section === "trending"
    readonly property bool skeletonShown: GifStore.configured && GifStore.loading && remoteFeed && GifStore.errorString === ""
    readonly property string statusMode: !GifStore.configured ? "unconfigured"
        : GifStore.errorString !== "" && currentModel.count === 0 ? "error"
        : currentModel.count === 0 && !(GifStore.loading && remoteFeed) ? (showingSearch ? "search" : section)
        : ""
    readonly property real spacing: Kirigami.Units.smallSpacing * 1.5

    readonly property var statusContent: ({
        unconfigured: { icon: "dialog-warning", title: qsTr("GIFs are not set up"), text: GifStore.errorString, action: "" },
        error: { icon: "network-disconnect", title: qsTr("Couldn't load GIFs"), text: GifStore.errorString, action: qsTr("Try again") },
        search: { icon: "search", title: qsTr("No results"), text: qsTr("Try a different word for “%1”").arg(effectiveQuery.trim()), action: "" },
        favorites: { icon: "emblem-favorite", title: qsTr("No favorites yet"), text: qsTr("Press and hold a GIF to keep it here"), action: "" },
        recents: { icon: "document-open-recent", title: qsTr("Nothing sent yet"), text: qsTr("GIFs you send show up here"), action: qsTr("Browse trending") },
        trending: { icon: "image-gif", title: qsTr("Nothing here"), text: "", action: qsTr("Try again") },
        category: { icon: "image-gif", title: qsTr("Nothing here"), text: "", action: qsTr("Try again") }
    })

    signal searchRequested()
    signal closeRequested()

    function appendSearch(text) {
        searchText += text
    }

    function backspaceSearch() {
        const chars = Array.from(searchText)
        chars.pop()
        searchText = chars.join("")
    }

    function selectSection(name, query) {
        section = name
        categoryQuery = query
        grid.contentY = 0
        if (name === "category" && searchText.trim() === "") {
            GifStore.searchNow(query)
        }
    }

    function loadIfShown() {
        if (visible && GifStore.configured) {
            GifStore.load()
        }
    }

    onEffectiveQueryChanged: {
        GifStore.query = effectiveQuery
        grid.contentY = 0
    }
    onVisibleChanged: loadIfShown()
    Component.onCompleted: {
        GifStore.query = effectiveQuery
        loadIfShown()
    }

    Item {
        id: header
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: Kirigami.Units.largeSpacing
        anchors.topMargin: Kirigami.Units.smallSpacing * 1.5
        height: Kirigami.Units.gridUnit * 2.2

        GifRoundButton {
            id: closeButton
            width: parent.height
            height: width
            iconName: "go-previous"
            animationScale: panel.animationScale
            onClicked: panel.closeRequested()
        }

        GifSearchPill {
            anchors.left: closeButton.right
            anchors.right: mediaToggle.left
            anchors.leftMargin: Kirigami.Units.smallSpacing * 1.5
            anchors.rightMargin: Kirigami.Units.smallSpacing * 1.5
            height: parent.height
            text: panel.searchText
            placeholder: GifStore.searchPlaceholder
            active: panel.searchActive
            animationScale: panel.animationScale
            onClicked: {
                panel.searchActive = true
                panel.searchRequested()
            }
            onClearClicked: panel.searchText = ""
        }

        GifMediaToggle {
            id: mediaToggle
            anchors.right: parent.right
            width: implicitWidth
            height: parent.height
            animationScale: panel.animationScale
        }
    }

    GifChipsRow {
        id: chips
        anchors.top: header.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.topMargin: Kirigami.Units.smallSpacing * 1.5
        height: Kirigami.Units.gridUnit * 1.6
        selectedSection: panel.section
        categoryQuery: panel.categoryQuery
        searchText: panel.searchText
        animationScale: panel.animationScale
        onSectionSelected: (name, query) => panel.selectSection(name, query)
        onSuggestionSelected: text => panel.searchText = text
    }

    GifGrid {
        id: grid
        anchors.top: chips.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.topMargin: Kirigami.Units.smallSpacing * 1.5
        leftMargin: Kirigami.Units.largeSpacing
        rightMargin: Kirigami.Units.largeSpacing
        bottomMargin: Kirigami.Units.gridUnit * 2
        gifModel: panel.currentModel
        columns: panel.columns
        spacing: panel.spacing
        remote: panel.remoteFeed
        active: panel.visible
        hidden: panel.skeletonShown
        animationScale: panel.animationScale
    }

    GifSkeleton {
        anchors.fill: grid
        anchors.leftMargin: Kirigami.Units.largeSpacing
        anchors.rightMargin: Kirigami.Units.largeSpacing
        columns: panel.columns
        spacing: panel.spacing
        opacity: panel.skeletonShown ? 1 : 0
        animationScale: panel.animationScale
    }

    GifStatus {
        anchors.fill: grid
        visible: panel.statusMode !== ""
        readonly property var content: panel.statusContent[panel.statusMode] || panel.statusContent.trending
        iconName: content.icon
        title: content.title
        text: content.text
        actionText: content.action
        animationScale: panel.animationScale
        onActionTriggered: panel.statusMode === "recents" ? panel.selectSection("trending", "") : GifStore.retry()
    }

    GifToast {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: Kirigami.Units.gridUnit * 2.2
        maximumWidth: parent.width - Kirigami.Units.gridUnit * 2
        text: GifStore.errorString
        shown: GifStore.configured && GifStore.errorString !== "" && panel.statusMode !== "error"
        animationScale: panel.animationScale
        onDismissed: GifStore.clearError()
    }

    Rectangle {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.rightMargin: Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing
        anchors.bottomMargin: Kirigami.Units.smallSpacing * 1.5
        width: attributionLabel.implicitWidth + Kirigami.Units.largeSpacing * 1.5
        height: Kirigami.Units.gridUnit * 1.3
        radius: height / 2
        color: Qt.alpha(Kirigami.Theme.backgroundColor, 0.88)
        border.width: 1
        border.color: Qt.alpha(Kirigami.Theme.textColor, 0.1)

        Text {
            id: attributionLabel
            anchors.centerIn: parent
            text: GifStore.attribution
            color: Qt.alpha(Kirigami.Theme.textColor, 0.8)
            font.pointSize: Kirigami.Theme.smallFont.pointSize
            font.weight: Font.DemiBold
        }
    }
}
