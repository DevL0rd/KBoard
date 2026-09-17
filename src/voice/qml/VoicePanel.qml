import QtQuick
import org.devl0rd.kboard.config
import org.devl0rd.kboard.voice

VoicePanelView {
    id: panel

    property bool autoStart: true
    signal closeRequested()

    voiceState: VoiceTyping.state
    paused: VoiceTyping.paused
    level: VoiceTyping.level
    partialText: VoiceTyping.partialText
    committedText: VoiceTyping.committedText
    errorString: VoiceTyping.errorString
    modelName: VoiceTyping.modelName
    modelVariant: VoiceTyping.modelVariant
    modelSizeMb: VoiceTyping.modelSizeMb
    languageCount: VoiceTyping.modelLanguages.length
    language: Settings.voiceLanguage
    backend: VoiceTyping.backend
    backendLabel: VoiceTyping.backendLabel
    downloadProgress: VoiceTyping.downloadProgress
    downloadError: VoiceTyping.downloadError
    commandsEnabled: Settings.voiceCommands

    function startIfIdle() {
        if (visible && autoStart && VoiceTyping.state === "idle")
            VoiceTyping.start()
    }

    function close() {
        VoiceTyping.cancel()
        closeRequested()
    }

    onVisibleChanged: startIfIdle()
    Component.onCompleted: startIfIdle()

    onOrbClicked: {
        if (needsModel)
            VoiceTyping.download(VoiceTyping.modelId)
        else if (listening)
            VoiceTyping.togglePause()
        else if (!busy)
            VoiceTyping.start()
    }
    onCloseClicked: close()
    onCancelClicked: close()
    onPauseClicked: VoiceTyping.togglePause()
    onDoneClicked: VoiceTyping.stop()
    onDownloadClicked: VoiceTyping.download(VoiceTyping.modelId)
    onStopDownloadClicked: VoiceTyping.cancelDownload()
    onRetryClicked: VoiceTyping.start()
}
