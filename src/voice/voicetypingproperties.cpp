#include "voicetyping.h"

#include "voicestates.h"

#include "audiocapture.h"
#include "kboardsettings.h"
#include "voicelogging.h"
#include "voicemodels.h"

QString VoiceTyping::state() const
{
    return m_state;
}

bool VoiceTyping::isPaused() const
{
    return m_paused;
}

double VoiceTyping::level() const
{
    return m_level;
}

QString VoiceTyping::partialText() const
{
    return m_partialText;
}

QString VoiceTyping::committedText() const
{
    return m_writer.committedText();
}

QString VoiceTyping::errorString() const
{
    return m_errorString;
}

QVariantList VoiceTyping::models() const
{
    return m_models->list(m_engine->loadedPath());
}

QString VoiceTyping::speedReference() const
{
    return m_models->speedReference();
}

QString VoiceTyping::modelId() const
{
    return KBoardSettings::voiceModel();
}

QString VoiceTyping::modelName() const
{
    const ModelEntry *entry = m_models->current();
    return entry ? entry->name : QString();
}

QString VoiceTyping::modelVariant() const
{
    const ModelEntry *entry = m_models->current();
    return entry ? entry->variant : QString();
}

int VoiceTyping::modelSizeMb() const
{
    const ModelEntry *entry = m_models->current();
    return entry ? entry->toVariantMap().value(QStringLiteral("sizeMb")).toInt() : 0;
}

QStringList VoiceTyping::modelLanguages() const
{
    const ModelEntry *entry = m_models->current();
    return entry ? entry->languages : QStringList();
}

bool VoiceTyping::isModelDownloaded() const
{
    const ModelEntry *entry = m_models->current();
    return entry && m_models->isReady(*entry);
}

bool VoiceTyping::isModelLoaded() const
{
    return m_engine->isReady();
}

double VoiceTyping::downloadProgress() const
{
    return m_models->progress();
}

QString VoiceTyping::downloadingId() const
{
    return m_models->downloadingId();
}

QString VoiceTyping::downloadError() const
{
    return m_models->downloadError();
}

QString VoiceTyping::backend() const
{
    return m_engine->isReady() ? m_engine->device().name : QString();
}

QString VoiceTyping::backendLabel() const
{
    return m_engine->isReady() ? m_engine->device().label() : QString();
}

QString VoiceTyping::backendDescription() const
{
    return m_engine->isReady() ? m_engine->device().description : QString();
}

QVariantList VoiceTyping::devices()
{
    if (!m_engine->hasDevices()) {
        m_engine->refreshDevices();
    }
    return m_engine->devices();
}

QVariantList VoiceTyping::microphones() const
{
    return AudioCapture::microphones();
}

QString VoiceTyping::microphone() const
{
    return m_microphone;
}

double VoiceTyping::realTimeFactor() const
{
    return m_realTimeFactor;
}

void VoiceTyping::setState(const QString &state)
{
    if (m_state != state) {
        m_state = state;
        Q_EMIT stateChanged();
    }
}

void VoiceTyping::setError(const QString &error)
{
    qCWarning(KBOARD_VOICE) << error;
    if (m_errorString != error) {
        m_errorString = error;
        Q_EMIT errorStringChanged();
    }
    setState(VoiceStates::error);
}

void VoiceTyping::setPartialText(const QString &text)
{
    if (m_partialText != text) {
        m_partialText = text;
        Q_EMIT partialTextChanged();
    }
}

void VoiceTyping::setLevel(double level)
{
    if (m_level != level) {
        m_level = level;
        Q_EMIT levelChanged();
    }
}

void VoiceTyping::setPaused(bool paused)
{
    if (m_paused != paused) {
        m_paused = paused;
        Q_EMIT pausedChanged();
    }
}
