pragma ComponentBehavior: Bound

import QtQuick
import org.kde.kirigami as Kirigami
import org.devl0rd.kboard.config
import org.devl0rd.kboard.clipboard
import org.devl0rd.kboard.sound

Item {
    id: keyboard

    property real baseHeight: 320
    property real screenHeight: 1080
    property string panel: "keys"
    property bool fullKeyboard: false
    property real slide: 0
    property real splitAmount: splitShown ? 1 : 0
    property bool shown: true
    property string application

    readonly property alias keys: keysView
    readonly property alias strip: strip
    readonly property alias typing: typing
    readonly property var currentPanel: host.current
    readonly property alias rules: appRules
    readonly property real stripHeight: Math.round(Math.max(Kirigami.Units.gridUnit * 2, Math.min(baseHeight * 0.15, Kirigami.Units.gridUnit * 3)))
    readonly property bool tall: panel === "emoji" || panel === "gif"
    readonly property real expandedHeight: Math.round(baseHeight * 1.25)
    readonly property real targetHeight: tall ? expandedHeight : baseHeight
    readonly property real headroom: Math.round(baseHeight * 0.12)
    readonly property int rowCount: Math.max(4, keysView.surface.rowCount)
    readonly property bool splitWanted: {
        if (Settings.splitMode !== 0)
            return Settings.splitMode === 1
        const keyWidth = width / 10
        const keyHeight = (baseHeight - stripHeight) / rowCount
        return width >= 2400 || (width / Math.max(1, screenHeight) >= 1.6 && keyWidth > keyHeight * 1.5)
    }
    readonly property bool splitShown: splitWanted && panel === "keys"
    readonly property real splitHalf: Math.round(width * Settings.splitHalfWidth)
    readonly property real halfWidth: splitAmount > 0 ? width / 2 - (width / 2 - splitHalf) * splitAmount : width / 2
    readonly property real panelTop: height - targetHeight
    readonly property var interactiveRects: splitAmount > 0
        ? [Qt.rect(0, panelTop, halfWidth, targetHeight), Qt.rect(width - halfWidth, panelTop, halfWidth, targetHeight)]
        : [Qt.rect(0, panelTop, width, targetHeight)]

    signal hideRequested
    signal settingsRequested

    function handleAction(action) {
        if (action === "hide") {
            hideRequested()
        } else if (action === "globe") {
            const ids = Settings.layouts
            switchLayout(ids[(ids.indexOf(Settings.activeLayout) + 1) % ids.length])
        } else if (action.startsWith("layout:")) {
            switchLayout(action.slice(7))
        } else if (action.startsWith("panel:")) {
            panel = action.slice(6)
        }
    }

    function describe() {
        const surface = keysView.surface
        const keys = []
        for (let i = 0; i < surface.count; ++i) {
            const rect = surface.keyRect(i)
            const point = surface.mapToItem(keyboard, rect.x, rect.y)
            const data = surface.keyData(i)
            keys.push({ label: data.label, type: data.type, key: data.key, x: point.x, y: point.y, width: rect.width, height: rect.height,
                        pressed: surface.model.data(surface.model.index(i, 0), Qt.UserRole + 11) === true })
        }
        const suggestions = [0, 1, 2].map(index => {
            const rect = strip.suggestionRect(index)
            const point = strip.mapToItem(keyboard, rect.x, rect.y)
            return { text: strip.suggestionMode ? strip.slots[index] : "", x: point.x, y: point.y, width: rect.width, height: rect.height }
        })
        return {
            panel: panel,
            page: keysView.pageId,
            layout: keysView.layoutId,
            shift: keysView.shiftState,
            split: splitAmount > 0,
            searching: host.searching,
            rects: interactiveRects.map(rect => ({ x: rect.x, y: rect.y, width: rect.width, height: rect.height })),
            keys: keys,
            suggestions: suggestions
        }
    }

    function switchLayout(id) {
        if (!id || id === Settings.activeLayout)
            return
        Settings.activeLayout = id
        SettingsWatcher.save()
    }

    function requestTool(name) {
        if (name === "full")
            fullKeyboard = !fullKeyboard
        else if (name === "settings")
            settingsRequested()
        else if (name === "hide")
            hideRequested()
        else
            panel = panel === name ? "keys" : name
    }

    height: expandedHeight + headroom

    Behavior on splitAmount {
        NumberAnimation { duration: Theme.duration(320); easing.type: Easing.OutCubic }
    }

    Item {
        id: panelArea

        property real displayHeight: keyboard.targetHeight

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: displayHeight
        opacity: 1 - keyboard.slide * 0.7
        transform: Translate {
            y: keyboard.slide * (panelArea.displayHeight + keyboard.headroom)
        }

        Behavior on displayHeight {
            NumberAnimation { duration: Theme.duration(240); easing.type: Easing.OutCubic }
        }

        KeyboardBackdrop {
            anchors.fill: parent
            halfWidth: keyboard.halfWidth
        }

        SuggestionStrip {
            id: strip

            anchors.left: parent.left
            anchors.right: parent.right
            height: keyboard.stripHeight
            split: keyboard.splitAmount > 0
            halfWidth: keyboard.halfWidth
            activePanel: keyboard.panel
            suggestions: typing.suggestions
            autocorrection: typing.autocorrection
            emojis: typing.emojis
            showSuggestions: typing.showSuggestions
            clipboardFresh: Settings.clipboardSuggestChip && ClipboardHistory.latestFresh
            clipboardEntry: ClipboardHistory.latest
            swipeToHide: Settings.hideOnSwipeDown
            onToolRequested: name => keyboard.requestTool(name)
            onClipboardChipTapped: ClipboardHistory.pasteLatest()
            onClipboardChipDismissed: ClipboardHistory.dismissLatest()
            onSwipedDown: keyboard.hideRequested()
        }

        PanelHost {
            id: host

            anchors.fill: parent
            anchors.topMargin: keyboard.stripHeight
            panel: keyboard.panel
            searchKeysHeight: (keyboard.baseHeight - keyboard.stripHeight) * 0.78
            onCloseRequested: keyboard.panel = "keys"

            KeyboardView {
                id: keysView

                anchors.fill: parent
                anchors.leftMargin: Theme.gap / 2
                anchors.rightMargin: Theme.gap / 2
                anchors.bottomMargin: Theme.gap / 2
                numberRow: Settings.showNumberRow || keyboard.fullKeyboard
                functionRow: keyboard.fullKeyboard
                desktopRow: Settings.showDesktopRow || keyboard.fullKeyboard || appRules.desktopRow || typing.field.terminal
                split: keyboard.splitAmount > 0
                splitGap: Math.max(0, (keyboard.width - keyboard.splitHalf * 2) * keyboard.splitAmount)
                onActionKey: (action, key) => keyboard.handleAction(action)
            }
        }
    }

    AppRules {
        id: appRules

        application: keyboard.application
    }

    TypingController {
        id: typing

        view: keysView
        strip: strip
        learningAllowed: !appRules.noLearn
    }

    ControllerNavigation {
        keyboard: keyboard
        shown: keyboard.shown
        keys: keysView
        panel: host.current
    }
}
