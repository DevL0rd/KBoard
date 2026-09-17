pragma ComponentBehavior: Bound

import QtQuick
import org.kde.kirigami as Kirigami
import org.devl0rd.kboard.config

Item {
    id: card

    required property int index
    required property string itemId
    required property string text
    required property string kind
    required property string thumbnailSource
    required property int imageWidth
    required property int imageHeight
    required property bool pinned
    required property string domain
    required property string otpCode
    required property bool expiring

    property bool shown: true
    property bool focused: false
    property bool leaving: false
    property bool animateLayout: false
    property real targetX: 0
    property real targetY: 0
    property int enterDelay: 0
    property bool entered: false
    property real maximumImageHeight: Kirigami.Units.gridUnit * 9

    readonly property real padding: Kirigami.Units.largeSpacing

    signal activated()
    signal pinToggled()
    signal removeRequested()

    function dismiss(direction) {
        if (leaving)
            return
        leaving = true
        slideOut.to = (direction < 0 ? -1 : 1) * (width + Kirigami.Units.gridUnit * 2)
        dismissAnimation.start()
    }

    Kirigami.Theme.colorSet: Kirigami.Theme.View
    Kirigami.Theme.inherit: false

    x: targetX
    y: targetY
    z: swipe.active || leaving ? 2 : (focused ? 1 : 0)
    implicitHeight: content.implicitHeight
    height: implicitHeight
    visible: opacity > 0
    opacity: shown && entered && !leaving ? (expiring ? 0.45 : 1) : 0
    scale: shown && entered ? 1 : 0.86

    Behavior on x { enabled: card.animateLayout && !card.leaving; NumberAnimation { duration: ClipboardStyle.duration(260); easing.type: Easing.OutCubic } }
    Behavior on y { enabled: card.animateLayout && !card.leaving; NumberAnimation { duration: ClipboardStyle.duration(260); easing.type: Easing.OutCubic } }
    Behavior on opacity { enabled: !dismissAnimation.running; NumberAnimation { duration: ClipboardStyle.duration(card.expiring ? 900 : 220); easing.type: Easing.OutCubic } }
    Behavior on scale { NumberAnimation { duration: ClipboardStyle.duration(240); easing.type: Easing.OutBack } }

    Timer {
        interval: card.enterDelay
        running: true
        onTriggered: card.entered = true
    }

    SequentialAnimation {
        id: dismissAnimation
        ParallelAnimation {
            NumberAnimation { id: slideOut; target: body; property: "x"; duration: ClipboardStyle.duration(200); easing.type: Easing.InCubic }
            NumberAnimation { target: card; property: "opacity"; to: 0; duration: ClipboardStyle.duration(200); easing.type: Easing.InCubic }
        }
        ScriptAction { script: card.removeRequested() }
    }

    Rectangle {
        readonly property real progress: card.width > 0 ? Math.min(1, Math.abs(body.x) / (card.width * 0.35)) : 0
        anchors.fill: parent
        radius: ClipboardStyle.cardRadius
        visible: progress > 0
        color: Qt.alpha(Kirigami.Theme.negativeTextColor, 0.1 + progress * 0.18)

        Kirigami.Icon {
            source: "edit-delete"
            width: Kirigami.Units.iconSizes.smallMedium
            height: width
            color: Kirigami.Theme.negativeTextColor
            isMask: true
            anchors.verticalCenter: parent.verticalCenter
            x: body.x > 0 ? Kirigami.Units.largeSpacing : parent.width - width - Kirigami.Units.largeSpacing
            opacity: parent.progress
            scale: 0.6 + parent.progress * 0.4
        }
    }

    Item {
        id: body
        width: card.width
        height: card.height
        scale: tap.pressed && !swipe.active ? 0.95 : 1

        Behavior on x { enabled: !swipe.active && !card.leaving; SpringAnimation { spring: 4; damping: 0.3; epsilon: 0.5 } }
        Behavior on scale { NumberAnimation { duration: ClipboardStyle.duration(tap.pressed ? 90 : 260); easing.type: tap.pressed ? Easing.OutQuad : Easing.OutBack } }

        Kirigami.ShadowedRectangle {
            anchors.fill: parent
            radius: ClipboardStyle.cardRadius
            color: tap.pressed ? Kirigami.ColorUtils.linearInterpolation(Kirigami.Theme.backgroundColor, ClipboardStyle.accentColor, 0.18) : Kirigami.Theme.backgroundColor
            border.width: card.focused ? 2 : 1
            border.color: card.focused ? ClipboardStyle.accentColor : card.pinned ? Qt.alpha(ClipboardStyle.accentColor, 0.6) : Qt.alpha(Kirigami.Theme.textColor, 0.1)
            shadow.size: swipe.active ? Kirigami.Units.gridUnit : Kirigami.Units.smallSpacing * 1.5
            shadow.yOffset: swipe.active ? 3 : 1
            shadow.color: Qt.alpha(Qt.darker(Kirigami.Theme.backgroundColor, 4), swipe.active ? 0.35 : 0.12)
            Behavior on color { ColorAnimation { duration: ClipboardStyle.duration(120) } }
        }

        Loader {
            id: content
            width: parent.width
            sourceComponent: card.kind === "image" ? imageComponent : card.kind === "url" ? linkComponent : card.kind === "otp" ? otpComponent : textComponent
        }

        PinBadge {
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.margins: Kirigami.Units.smallSpacing * 1.5
            pinned: card.pinned
        }

        Kirigami.Icon {
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.margins: Kirigami.Units.smallSpacing * 1.5
            width: Kirigami.Units.iconSizes.small
            height: width
            source: "chronometer"
            color: Kirigami.Theme.disabledTextColor
            isMask: true
            visible: card.expiring && !card.pinned && card.kind !== "image"
        }
    }

    TapHandler {
        id: tap
        enabled: !card.leaving
        longPressThreshold: Settings.longPressDelay / 1000
        onTapped: card.activated()
        onLongPressed: card.pinToggled()
    }

    DragHandler {
        id: swipe
        property real startX: 0
        enabled: !card.leaving
        target: null
        yAxis.enabled: false
        grabPermissions: PointerHandler.CanTakeOverFromHandlersOfDifferentType | PointerHandler.ApprovesTakeOverByAnything
        onTranslationChanged: {
            if (active)
                body.x = startX + translation.x
        }
        onActiveChanged: {
            if (active) {
                startX = body.x
                return
            }
            const fling = Math.abs(centroid.velocity.x) > 900 && Math.abs(body.x) > Kirigami.Units.gridUnit
            if (Math.abs(body.x) > card.width * 0.35 || fling)
                card.dismiss(body.x)
            else
                body.x = 0
        }
    }

    Component {
        id: textComponent
        CardTextContent {
            text: card.text
            padding: card.padding
            trailingSpace: card.pinned || card.expiring ? Kirigami.Units.gridUnit : 0
        }
    }

    Component {
        id: imageComponent
        CardImageContent {
            source: card.thumbnailSource
            imageWidth: card.imageWidth
            imageHeight: card.imageHeight
            maximumHeight: card.maximumImageHeight
        }
    }

    Component {
        id: linkComponent
        CardLinkContent {
            text: card.text
            domain: card.domain
            padding: card.padding
        }
    }

    Component {
        id: otpComponent
        CardOtpContent {
            text: card.text
            code: card.otpCode
            padding: card.padding
        }
    }
}
