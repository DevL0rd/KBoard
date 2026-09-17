#include "speechengine.h"

#include "voicelogging.h"

#include <parakeet.h>
#include <whisper.h>

#include <QElapsedTimer>
#include <QFileInfo>
#include <QThread>

#include <mutex>

namespace
{
std::mutex s_logMutex;
QStringList s_recentErrors;

void logCallback(ggml_log_level level, const char *text, void *)
{
    const bool important = level == GGML_LOG_LEVEL_ERROR || level == GGML_LOG_LEVEL_WARN;
    if (!important && !KBOARD_VOICE().isDebugEnabled()) {
        return;
    }
    const QString message = QString::fromUtf8(text).trimmed();
    if (message.isEmpty()) {
        return;
    }
    if (important) {
        qCWarning(KBOARD_VOICE) << message;
        if (level == GGML_LOG_LEVEL_ERROR) {
            std::lock_guard lock(s_logMutex);
            s_recentErrors.append(message);
            while (s_recentErrors.size() > 4) {
                s_recentErrors.removeFirst();
            }
        }
        return;
    }
    qCDebug(KBOARD_VOICE) << message;
}

void clearRecentErrors()
{
    std::lock_guard lock(s_logMutex);
    s_recentErrors.clear();
}

QString recentErrors()
{
    std::lock_guard lock(s_logMutex);
    return s_recentErrors.join(QStringLiteral("; "));
}

QString withDetails(const QString &message)
{
    const QString details = recentErrors();
    return details.isEmpty() ? message : QStringLiteral("%1: %2").arg(message, details);
}

const int s_minimumSamples = 16000;
}

SpeechEngine::SpeechEngine() = default;

SpeechEngine::~SpeechEngine()
{
    unload();
}

void SpeechEngine::installLogHandler()
{
    static std::once_flag once;
    std::call_once(once, [] {
        ggml_log_set(logCallback, nullptr);
        whisper_log_set(logCallback, nullptr);
        parakeet_log_set(logCallback, nullptr);
    });
}

int SpeechEngine::threadCount()
{
    return qBound(1, QThread::idealThreadCount() / 2, 8);
}

bool SpeechEngine::load(const QString &engine, const QString &modelPath, const QString &deviceSetting, QString *error)
{
    unload();
    installLogHandler();
    clearRecentErrors();

    const auto fail = [error](const QString &message) {
        if (error) {
            *error = message;
        }
        return false;
    };

    if (engine != u"parakeet" && engine != u"whisper") {
        return fail(QStringLiteral("Unknown speech engine \"%1\"").arg(engine));
    }
    if (!QFileInfo(modelPath).isFile()) {
        return fail(QStringLiteral("Voice model file is missing: %1").arg(modelPath));
    }

    QString selectError;
    const VoiceDevice device = VoiceBackends::select(VoiceBackends::devices(), deviceSetting, &selectError);
    if (device.name.isEmpty()) {
        return fail(selectError);
    }
    QString probeError;
    if (!VoiceBackends::probe(device, &probeError)) {
        return fail(withDetails(probeError));
    }

    const QByteArray path = modelPath.toUtf8();
    if (engine == u"parakeet") {
        parakeet_context_params params = parakeet_context_default_params();
        params.use_gpu = device.isGpu();
        params.gpu_device = qMax(0, device.gpuIndex);
        m_parakeet = parakeet_init_from_file_with_params(path.constData(), params);
        if (!m_parakeet) {
            return fail(withDetails(QStringLiteral("Could not load %1 on %2").arg(QFileInfo(modelPath).fileName(), device.label())));
        }
    } else {
        whisper_context_params params = whisper_context_default_params();
        params.use_gpu = device.isGpu();
        params.gpu_device = qMax(0, device.gpuIndex);
        m_whisper = whisper_init_from_file_with_params(path.constData(), params);
        if (!m_whisper) {
            return fail(withDetails(QStringLiteral("Could not load %1 on %2").arg(QFileInfo(modelPath).fileName(), device.label())));
        }
    }
    m_device = device;
    qCInfo(KBOARD_VOICE) << "Loaded" << modelPath << "on" << device.name << device.description;
    return true;
}

void SpeechEngine::unload()
{
    if (m_parakeet) {
        parakeet_free(m_parakeet);
        m_parakeet = nullptr;
    }
    if (m_whisper) {
        whisper_free(m_whisper);
        m_whisper = nullptr;
    }
    m_device = {};
}

bool SpeechEngine::isLoaded() const
{
    return m_parakeet || m_whisper;
}

VoiceDevice SpeechEngine::device() const
{
    return m_device;
}

SpeechEngine::Result SpeechEngine::transcribe(const QList<float> &samples, const QString &language)
{
    Result result;
    if (!isLoaded()) {
        result.error = QStringLiteral("No voice model is loaded");
        return result;
    }
    clearRecentErrors();

    QList<float> audio = samples;
    if (audio.size() < s_minimumSamples) {
        audio.resize(s_minimumSamples, 0.0f);
    }
    result.audioSeconds = double(samples.size()) / 16000.0;

    QElapsedTimer timer;
    timer.start();
    QString text;
    if (m_parakeet) {
        parakeet_full_params params = parakeet_full_default_params(PARAKEET_SAMPLING_GREEDY);
        params.n_threads = threadCount();
        params.no_context = true;
        if (parakeet_full(m_parakeet, params, audio.constData(), int(audio.size())) != 0) {
            result.error = withDetails(QStringLiteral("Speech recognition failed on %1").arg(m_device.label()));
            return result;
        }
        const int segments = parakeet_full_n_segments(m_parakeet);
        for (int i = 0; i < segments; ++i) {
            text += QString::fromUtf8(parakeet_full_get_segment_text(m_parakeet, i));
        }
    } else {
        whisper_full_params params = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);
        const QByteArray lang = (language.isEmpty() ? QStringLiteral("auto") : language).toUtf8();
        params.n_threads = threadCount();
        params.no_context = true;
        params.no_timestamps = true;
        params.single_segment = false;
        params.print_progress = false;
        params.print_realtime = false;
        params.print_special = false;
        params.print_timestamps = false;
        params.suppress_nst = true;
        params.language = lang.constData();
        if (whisper_full(m_whisper, params, audio.constData(), int(audio.size())) != 0) {
            result.error = withDetails(QStringLiteral("Speech recognition failed on %1").arg(m_device.label()));
            return result;
        }
        const int segments = whisper_full_n_segments(m_whisper);
        for (int i = 0; i < segments; ++i) {
            text += QString::fromUtf8(whisper_full_get_segment_text(m_whisper, i));
        }
    }
    result.decodeSeconds = double(timer.nsecsElapsed()) / 1e9;
    result.text = text.simplified();
    result.ok = true;
    return result;
}
