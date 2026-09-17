import QtQuick
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Item {
    id: view

    property string voiceState: "idle"
    property bool paused: false
    property real level: 0
    property string partialText
    property string committedText
    property string errorString
    property string modelName
    property string modelVariant
    property int modelSizeMb: 0
    property int languageCount: 0
    property string language: "auto"
    property string backend
    property string backendLabel
    property real downloadProgress: -1
    property string downloadError
    property bool commandsEnabled: true

    signal orbClicked()
    signal closeClicked()
    signal cancelClicked()
    signal pauseClicked()
    signal doneClicked()
    signal downloadClicked()
    signal stopDownloadClicked()
    signal retryClicked()

    readonly property bool listening: voiceState === "listening"
    readonly property bool busy: voiceState === "loading" || voiceState === "processing"
    readonly property bool needsModel: voiceState === "needs-model"
    readonly property bool failed: voiceState === "error"
    readonly property bool downloading: downloadProgress >= 0
    readonly property bool wide: width > height * 1.45
    readonly property string orbMode: {
        if (needsModel)
            return "download"
        if (failed)
            return "error"
        if (busy)
            return "busy"
        if (listening)
            return paused ? "paused" : "listening"
        return "idle"
    }

    property real smoothLevel: 0
    property real phase: 0
    property real spin: 0

    function sizeText(mb) {
        return mb >= 1024 ? i18nc("@info file size", "%1 GB", (mb / 1024).toFixed(1)) : i18nc("@info file size", "%1 MB", mb)
    }

    function title() {
        if (downloading)
            return i18nc("@info", "Downloading voice model…")
        const titles = {
            "needs-model": i18nc("@info", "Download the voice model to start"),
            "error": i18nc("@info", "Voice typing hit a problem"),
            "loading": i18nc("@info", "Loading model…"),
            "processing": i18nc("@info", "Finishing up…"),
            "listening": paused ? i18nc("@info", "Paused") : i18nc("@info", "Listening…")
        }
        return titles[voiceState] ?? i18nc("@info", "Tap the mic to talk")
    }

    FrameAnimation {
        running: view.visible && VoiceStyle.animate
        onTriggered: {
            const dt = Math.min(frameTime, 0.05) * VoiceStyle.speed
            const target = view.listening && !view.paused ? view.level : 0
            const rate = target > view.smoothLevel ? 18 : 7
            view.smoothLevel += (target - view.smoothLevel) * (1 - Math.exp(-dt * rate))
            view.phase = (view.phase + dt * (0.45 + view.smoothLevel * 0.9)) % 1
            view.spin = (view.spin + dt * 300) % 360
            waveform.push(view.smoothLevel, dt)
        }
    }

    Binding {
        target: view
        property: "smoothLevel"
        value: view.listening && !view.paused ? view.level : 0
        when: !VoiceStyle.animate
    }

    RowLayout {
        id: header

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            margins: Kirigami.Units.largeSpacing
            leftMargin: Kirigami.Units.largeSpacing * 2
        }
        spacing: Kirigami.Units.smallSpacing
        z: 2

        VoiceChip {
            visible: view.modelName !== ""
            iconName: "globe-symbolic"
            text: {
                const languages = view.language === "auto" ? i18ncp("@info", "%1 language", "%1 languages", view.languageCount) : view.language.toUpperCase()
                return i18nc("@info model name, variant, languages", "%1 %2  ·  %3", view.modelName, view.modelVariant, languages)
            }
        }

        VoiceChip {
            visible: view.backendLabel !== ""
            iconName: view.backend === "CPU" ? "computer-symbolic" : "show-gpu-effects-symbolic"
            text: view.backendLabel
            highlighted: true
        }

        Item {
            Layout.fillWidth: true
        }

        VoiceButton {
            round: true
            implicitHeight: Kirigami.Units.gridUnit * 2
            iconName: "arrow-down-symbolic"
            text: i18nc("@action:button", "Close voice typing")
            onClicked: view.closeClicked()
        }
    }

    Item {
        id: stage

        anchors {
            top: header.bottom
            left: parent.left
            right: parent.right
            bottom: controls.top
            leftMargin: Kirigami.Units.largeSpacing * 2
            rightMargin: Kirigami.Units.largeSpacing * 2
        }

        Item {
            id: orbSlot
            width: view.wide ? Math.min(stage.height * 1.2, stage.width * 0.34) : stage.width
            height: view.wide ? stage.height : Math.min(stage.height * 0.52, stage.width * 0.6)
        }

        VoiceOrb {
            readonly property real diameter: Math.max(Kirigami.Units.gridUnit * 3, Math.min(orbSlot.width, orbSlot.height) * 0.5)
            width: diameter
            height: diameter
            x: (orbSlot.width - width) / 2
            y: (orbSlot.height - height) / 2
            mode: view.orbMode
            level: view.smoothLevel
            phase: view.phase
            spin: view.spin
            progress: view.downloading ? view.downloadProgress : -1
            onClicked: view.orbClicked()
        }

        ColumnLayout {
            anchors {
                left: view.wide ? orbSlot.right : parent.left
                right: parent.right
                top: view.wide ? parent.top : orbSlot.bottom
                bottom: parent.bottom
                leftMargin: view.wide ? Kirigami.Units.largeSpacing * 2 : 0
            }
            spacing: Kirigami.Units.smallSpacing

            Item { Layout.fillHeight: true }

            VoiceStatusLine {
                Layout.fillWidth: true
                title: view.title()
                failed: view.failed
                pulsing: view.listening && !view.paused
                showDot: view.listening || view.busy || view.failed
                percent: view.downloading ? Math.round(view.downloadProgress * 100) : -1
                centered: !view.wide
                phase: view.phase
            }

            VoiceTranscript {
                Layout.fillWidth: true
                Layout.maximumHeight: stage.height * 0.7
                visible: !view.needsModel && !view.failed
                committed: view.committedText
                partial: view.partialText
                centered: !view.wide
                fontSize: Math.max(Kirigami.Theme.defaultFont.pixelSize * 1.5, Math.min(Kirigami.Theme.defaultFont.pixelSize * 2.3, stage.height * 0.16))
                placeholder: view.listening && !view.paused
                    ? (view.commandsEnabled ? i18nc("@info", "Start speaking. Say “new line”, “comma” or “delete that”.") : i18nc("@info", "Start speaking."))
                    : ""
            }

            VoiceMessage {
                Layout.fillWidth: true
                visible: view.needsModel
                large: true
                centered: !view.wide
                text: i18nc("@info", "%1 %2 runs privately on this computer. It is a one-time %3 download.", view.modelName, view.modelVariant, view.sizeText(view.modelSizeMb))
            }

            VoiceMessage {
                Layout.fillWidth: true
                visible: text !== ""
                centered: !view.wide
                large: view.failed
                danger: !view.failed
                text: view.failed ? view.errorString : (view.needsModel ? view.downloadError : "")
            }

            VoiceWaveform {
                id: waveform
                Layout.preferredWidth: Math.min(parent.width, Kirigami.Units.gridUnit * 14)
                Layout.alignment: view.wide ? Qt.AlignLeft : Qt.AlignHCenter
                Layout.preferredHeight: Kirigami.Units.gridUnit * 1.6
                Layout.topMargin: Kirigami.Units.smallSpacing
                visible: view.listening && VoiceStyle.animate
                opacity: view.paused ? 0.3 : 1
            }

            Item { Layout.fillHeight: true }
        }
    }

    RowLayout {
        id: controls

        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
            margins: Kirigami.Units.largeSpacing
            leftMargin: Kirigami.Units.largeSpacing * 2
            rightMargin: Kirigami.Units.largeSpacing * 2
        }
        spacing: Kirigami.Units.largeSpacing

        VoiceButton {
            text: i18nc("@action:button", "Cancel")
            iconName: "dialog-cancel-symbolic"
            onClicked: view.cancelClicked()
        }

        Item {
            Layout.fillWidth: true
        }

        VoiceButton {
            visible: view.listening
            text: view.paused ? i18nc("@action:button", "Resume") : i18nc("@action:button", "Pause")
            iconName: view.paused ? "media-playback-start-symbolic" : "media-playback-pause-symbolic"
            onClicked: view.pauseClicked()
        }

        VoiceButton {
            visible: view.needsModel
            primary: !view.downloading
            text: view.downloading ? i18nc("@action:button", "Stop download") : i18nc("@action:button", "Download %1", view.sizeText(view.modelSizeMb))
            iconName: view.downloading ? "media-playback-stop-symbolic" : "download-symbolic"
            onClicked: view.downloading ? view.stopDownloadClicked() : view.downloadClicked()
        }

        VoiceButton {
            visible: view.failed
            primary: true
            text: i18nc("@action:button", "Try again")
            iconName: "view-refresh-symbolic"
            onClicked: view.retryClicked()
        }

        VoiceButton {
            visible: !view.needsModel && !view.failed
            primary: true
            enabled: view.listening || view.voiceState === "loading"
            text: i18nc("@action:button", "Done")
            iconName: "checkmark-symbolic"
            onClicked: view.doneClicked()
        }
    }
}
