#include "silerovad.h"

#include "speechengine.h"
#include "voicebackends.h"

#include <whisper.h>

#include <QFileInfo>

SileroVad::~SileroVad()
{
    unload();
}

bool SileroVad::load(const QString &path, QString *error)
{
    if (m_context && m_path == path) {
        return true;
    }
    unload();
    if (!QFileInfo(path).isFile()) {
        if (error) {
            *error = QStringLiteral("Voice activity model is missing: %1").arg(path);
        }
        return false;
    }
    SpeechEngine::installLogHandler();
    VoiceBackends::ensureLoaded();
    whisper_vad_context_params params = whisper_vad_default_context_params();
    params.n_threads = 1;
    params.use_gpu = false;
    m_context = whisper_vad_init_from_file_with_params(path.toUtf8().constData(), params);
    if (!m_context) {
        if (error) {
            *error = QStringLiteral("Could not load the voice activity model %1").arg(path);
        }
        return false;
    }
    m_path = path;
    whisper_vad_reset_state(m_context);
    return true;
}

void SileroVad::unload()
{
    if (m_context) {
        whisper_vad_free(m_context);
        m_context = nullptr;
    }
    m_path.clear();
}

float SileroVad::probability(const float *window, int count)
{
    if (!m_context || !whisper_vad_detect_speech_no_reset(m_context, window, count) || whisper_vad_n_probs(m_context) < 1) {
        return 0.0f;
    }
    return whisper_vad_probs(m_context)[0];
}

void SileroVad::resetState()
{
    if (m_context) {
        whisper_vad_reset_state(m_context);
    }
}

int SileroVad::windowSamples() const
{
    return 512;
}
