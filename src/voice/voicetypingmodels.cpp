#include "voicetyping.h"

#include "voicestates.h"

#include "audiocapture.h"
#include "kboardsettings.h"
#include "voicemodels.h"

void VoiceTyping::scheduleUnload()
{
    if (m_session.active || !m_engine->isBusy()) {
        return;
    }
    m_unloadTimer.start(KBoardSettings::voiceUnloadMinutes() * 60 * 1000);
}

void VoiceTyping::unloadModel()
{
    if (m_session.active) {
        return;
    }
    m_unloadTimer.stop();
    m_engine->unload();
    QMetaObject::invokeMethod(m_capture, &AudioCapture::unloadVad, Qt::QueuedConnection);
    Q_EMIT backendChanged();
    Q_EMIT modelsChanged();
}

void VoiceTyping::onModelSettingsChanged()
{
    Q_EMIT modelChanged();
    Q_EMIT modelsChanged();
    if (!m_session.active && m_engine->isBusy()) {
        unloadModel();
    }
    if (m_state == VoiceStates::needsModel || m_state == VoiceStates::error) {
        setState(VoiceStates::idle);
    }
}

void VoiceTyping::download(const QString &id)
{
    m_models->download(id);
}

void VoiceTyping::cancelDownload()
{
    m_models->cancelDownload();
}

bool VoiceTyping::deleteModel(const QString &id)
{
    const ModelEntry *entry = m_models->find(id);
    if (entry && m_engine->isBusy() && m_engine->matches(m_models->pathFor(*entry), KBoardSettings::voiceDevice())) {
        if (m_session.active) {
            m_models->setDownloadError(QStringLiteral("Stop voice typing before deleting the model in use"));
            return false;
        }
        unloadModel();
    }
    return m_models->remove(id);
}

void VoiceTyping::refreshDevices()
{
    m_engine->refreshDevices();
}
