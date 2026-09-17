pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.devl0rd.kboard.settings

QQC2.StackView {
    id: host

    property var page: ({})
    property string errorString

    function open() {
        if (!page.type) {
            return;
        }
        const component = Qt.createComponent("org.devl0rd.kboard.settings", page.type);
        if (component.status !== Component.Ready) {
            errorString = component.errorString();
            console.warn("KBoard settings page", page.type, errorString);
            replace(null, errorPage, { text: errorString }, QQC2.StackView.Immediate);
            return;
        }
        errorString = "";
        replace(null, component, { pageId: page.id }, depth === 0 || !Motion.enabled ? QQC2.StackView.Immediate : QQC2.StackView.ReplaceTransition);
        revealTimer.restart();
    }

    function collect(item, text, found) {
        if (!item || !item.visible) {
            return found;
        }
        if (item["label"] === text || item["title"] === text) {
            found.push(item);
        }
        const children = item.children || [];
        for (let i = 0; i < children.length; ++i) {
            collect(children[i], text, found);
        }
        return found;
    }

    function reveal() {
        const item = currentItem as SettingsPage;
        if (!item || AppNavigation.revealLabel === "") {
            return;
        }
        const flick = item.flickable;
        const target = collect(flick.contentItem, AppNavigation.revealLabel, [])[0];
        if (!target) {
            console.warn("KBoard settings: no row labelled", AppNavigation.revealLabel, "on", page.id);
            return;
        }
        const spot = target.mapToItem(flick.contentItem, 0, 0);
        const destination = Math.max(0, Math.min(spot.y - flick.height * 0.25, flick.contentHeight - flick.height));
        scrollAnimation.target = flick;
        scrollAnimation.to = destination;
        scrollAnimation.restart();
        flash.createObject(flick.contentItem, {
            x: spot.x - Kirigami.Units.smallSpacing,
            y: spot.y - Kirigami.Units.smallSpacing,
            width: target.width + Kirigami.Units.smallSpacing * 2,
            height: target.height + Kirigami.Units.smallSpacing * 2
        });
        const focusable = firstFocusable(target);
        if (focusable) {
            focusable.forceActiveFocus(Qt.TabFocusReason);
        }
    }

    function firstFocusable(item) {
        if (!item) {
            return null;
        }
        if (item.activeFocusOnTab && item.enabled && item.visible) {
            return item;
        }
        const children = item.children || [];
        for (let i = 0; i < children.length; ++i) {
            const found = firstFocusable(children[i]);
            if (found) {
                return found;
            }
        }
        return null;
    }

    clip: true
    onPageChanged: open()
    Component.onCompleted: open()

    Connections {
        target: AppNavigation
        function onRevealRequested() {
            revealTimer.restart();
        }
    }

    Timer {
        id: revealTimer
        interval: 160
        onTriggered: host.reveal()
    }

    NumberAnimation {
        id: scrollAnimation
        property: "contentY"
        duration: Motion.medium
        easing.type: Easing.OutCubic
    }

    Component {
        id: flash
        Rectangle {
            id: flashRect
            z: 100
            radius: Kirigami.Units.cornerRadius * 2
            color: Qt.alpha(Kirigami.Theme.highlightColor, 0.12)
            border.width: 2
            border.color: Qt.alpha(Kirigami.Theme.highlightColor, 0.85)
            opacity: 0
            SequentialAnimation {
                running: true
                PauseAnimation { duration: Motion.enabled ? 180 : 0 }
                NumberAnimation { target: flashRect; property: "opacity"; to: 1; duration: Motion.ms(220); easing.type: Easing.OutCubic }
                PauseAnimation { duration: 1100 }
                NumberAnimation { target: flashRect; property: "opacity"; to: 0; duration: Motion.ms(700); easing.type: Easing.InOutCubic }
                ScriptAction { script: flashRect.destroy() }
            }
        }
    }

    Component {
        id: errorPage
        Kirigami.PlaceholderMessage {
            property string pageId
            icon.name: "dialog-error"
            text: "This page couldn't be loaded"
            explanation: host.errorString
            type: Kirigami.PlaceholderMessage.Type.Actionable
        }
    }

    replaceEnter: Transition {
        ParallelAnimation {
            NumberAnimation { property: "opacity"; from: 0; to: 1; duration: Motion.medium; easing.type: Easing.OutCubic }
            NumberAnimation { property: "y"; from: Kirigami.Units.gridUnit; to: 0; duration: Motion.medium; easing.type: Easing.OutCubic }
        }
    }

    replaceExit: Transition {
        NumberAnimation { property: "opacity"; from: 1; to: 0; duration: Motion.quick; easing.type: Easing.InCubic }
    }
}
