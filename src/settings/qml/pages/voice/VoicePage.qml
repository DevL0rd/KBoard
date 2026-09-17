pragma ComponentBehavior: Bound

import QtQuick
import org.devl0rd.kboard.config
import org.devl0rd.kboard.settings

SettingsPage {
    id: page

    readonly property var voice: Modules.voice ? Modules.voice.api : null
    readonly property var activeModel: voice ? voice.models.find(model => model.id === Settings.voiceModel) : null

    preview: VoicePreview {}

    ModuleNotice {
        module: "voice"
        what: "Voice typing"
    }

    Section {
        title: "Voice typing"

        SwitchRow {
            label: "Voice typing"
            description: "Speak instead of typing. Speech is turned into text on this computer and never uploaded."
            iconName: "audio-input-microphone"
            setting: "voiceEnabled"
        }

        ComboRow {
            label: "Language"
            description: "Automatic detects the language as you speak"
            iconName: "languages"
            setting: "voiceLanguage"
            enabled: Settings.voiceEnabled
            options: [{ value: "auto", label: "Automatic" }].concat((page.activeModel ? page.activeModel.languages : []).map(code => ({ value: code, label: Qt.locale(code).nativeLanguageName || code })))
        }

        SwitchRow {
            label: "Voice commands"
            description: "Say “new line”, “period”, “comma”, “question mark” or “delete that”"
            iconName: "irc-voice"
            setting: "voiceCommands"
            enabled: Settings.voiceEnabled
        }

        SwitchRow {
            label: "Stop when you stop talking"
            iconName: "media-playback-stop"
            setting: "voiceAutoStop"
            enabled: Settings.voiceEnabled
        }

        SliderRow {
            label: "Pause that ends a sentence"
            description: "How long a silence counts as the end of what you said"
            iconName: "chronometer"
            setting: "voiceEndSilenceMs"
            enabled: Settings.voiceEnabled
            from: 300
            to: 3000
            stepSize: 50
            unit: "ms"
        }
    }

    Section {
        title: "Speech model"

        ModelPicker {
            label: "Speech model"
            description: "Bigger models are more accurate but need more memory and time"
            iconName: "run-build"
            setting: "voiceModel"
        }
    }

    Section {
        title: "Hardware"

        ComboRow {
            label: "Processing device"
            description: page.voice && page.voice.backendDescription.length > 0 ? "Now using " + page.voice.backendDescription : "Automatic picks the fastest device that works on this computer"
            iconName: "cpu"
            setting: "voiceDevice"
            enabled: Settings.voiceEnabled
            options: [{ value: "auto", label: "Automatic" }].concat((page.voice ? page.voice.devices : []).map(device => ({ value: device.name, label: device.label })))
        }

        ComboRow {
            label: "Microphone"
            iconName: "audio-input-microphone"
            setting: "voiceMicrophone"
            enabled: Settings.voiceEnabled
            options: [{ value: "", label: "System default" }].concat((page.voice ? page.voice.microphones : []).map(mic => ({ value: mic.id, label: mic.description })))
        }

        SliderRow {
            label: "Free memory after"
            description: "Unload the model when voice typing hasn't been used for this long"
            iconName: "memory"
            setting: "voiceUnloadMinutes"
            enabled: Settings.voiceEnabled
            from: 1
            to: 60
            unit: "min"
        }
    }

    Section {
        title: "Try it"

        MicTestRow {
            label: "Microphone test"
            description: "Check that your microphone works and see how well the model understands you"
            iconName: "audio-input-microphone"
        }
    }
}
