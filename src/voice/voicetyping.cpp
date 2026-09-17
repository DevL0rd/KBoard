#include "voicetyping.h"

#include "voicestates.h"

#include "audiocapture.h"
#include "inputcontext.h"
#include "kboardsettings.h"
#include "voicelogging.h"
#include "voicemodels.h"

#include <QCoreApplication>
#include <QMediaDevices>
#include <QThread>

VoiceTyping *VoiceTyping::instance()
{
    static VoiceTyping *self = new VoiceTyping(QCoreApplication::instance());
    return self;
}

VoiceTyping *VoiceTyping::create(QQmlEngine *, QJSEngine *)
{
    VoiceTyping *self = instance();
    QJSEngine::setObjectOwnership(self, QJSEngine::CppOwnership);
    return self;
}

VoiceTyping::VoiceTyping(QObject *parent)
    : QObject(parent)
    , m_models(new VoiceModels(this))
    , m_engine(new EngineClient(this))
    , m_captureThread(new QThread(this))
    , m_capture(new AudioCapture)
    , m_mediaDevices(new QMediaDevices(this))
    , m_writer(InputContext::instance())
{
    m_captureThread->setObjectName(QStringLiteral("KBoard voice capture"));
    m_capture->moveToThread(m_captureThread);
    m_captureThread->start();

    connectCapture();
    connectSpeech();
    connectEngine();
    connectModels();
    connectSettings();

    m_unloadTimer.setSingleShot(true);
    connect(&m_unloadTimer, &QTimer::timeout, this, &VoiceTyping::unloadModel);
    connect(m_mediaDevices, &QMediaDevices::audioInputsChanged, this, &VoiceTyping::microphonesChanged);
    connect(InputContext::instance(), &InputContext::deactivated, this, &VoiceTyping::cancel);

    if (!m_models->catalogError().isEmpty()) {
        setError(m_models->catalogError());
    }
}

VoiceTyping::~VoiceTyping()
{
    QMetaObject::invokeMethod(m_capture, [capture = m_capture]() { delete capture; }, Qt::BlockingQueuedConnection);
    m_captureThread->quit();
    m_captureThread->wait();
}

bool VoiceTyping::isCurrent(quint64 session) const
{
    return session == m_session.id && m_session.active;
}

void VoiceTyping::connectCapture()
{
    connect(m_capture, &AudioCapture::started, this, [this](quint64 session, const QString &microphone) {
        if (isCurrent(session)) {
            m_session.capturing = true;
            m_microphone = microphone;
            Q_EMIT microphoneChanged();
            updateActiveState();
        }
    });
    connect(m_capture, &AudioCapture::failed, this, [this](quint64 session, const QString &error) {
        if (isCurrent(session)) {
            failSession(error);
        }
    });
    connect(m_capture, &AudioCapture::levelChanged, this, [this](quint64 session, double level) {
        if (isCurrent(session)) {
            setLevel(level);
        }
    });
    connect(m_capture, &AudioCapture::finished, this, [this](quint64 session) {
        if (isCurrent(session)) {
            m_session.capturing = false;
            finishIfDone();
        }
    });
}

void VoiceTyping::connectSpeech()
{
    connect(m_capture, &AudioCapture::speechStarted, this, [this](quint64 session, quint64 utterance) {
        if (isCurrent(session)) {
            m_session.liveUtterance = utterance;
        }
    });
    connect(m_capture, &AudioCapture::partialAudio, this, [this](quint64 session, quint64 utterance, const QList<float> &samples) {
        if (isCurrent(session) && utterance > m_session.finalizedUtterance) {
            m_engine->transcribe(DecodeRequest {session, utterance, false, samples, KBoardSettings::voiceLanguage()});
        }
    });
    connect(m_capture, &AudioCapture::utteranceAudio, this,
        [this](quint64 session, quint64 utterance, const QList<float> &samples, bool endOfSpeech) {
            if (isCurrent(session)) {
                onUtterance(utterance, samples, endOfSpeech);
            }
        });
    connect(m_capture, &AudioCapture::utteranceDiscarded, this, [this](quint64 session, quint64 utterance) {
        if (isCurrent(session) && utterance == m_session.liveUtterance) {
            setPartialText(QString());
            InputContext::instance()->clearPreedit();
        }
    });
}

void VoiceTyping::connectEngine()
{
    connect(m_engine, &EngineClient::readyChanged, this, [this]() {
        Q_EMIT backendChanged();
        Q_EMIT modelsChanged();
        updateActiveState();
        scheduleUnload();
    });
    connect(m_engine, &EngineClient::loadFailed, this, &VoiceTyping::failSession);
    connect(m_engine, &EngineClient::decoded, this, &VoiceTyping::onDecoded);
    connect(m_engine, &EngineClient::devicesChanged, this, &VoiceTyping::devicesChanged);
}

void VoiceTyping::connectModels()
{
    connect(m_models, &VoiceModels::changed, this, &VoiceTyping::modelsChanged);
    connect(m_models, &VoiceModels::progressChanged, this, &VoiceTyping::downloadProgressChanged);
    connect(m_models, &VoiceModels::downloadErrorChanged, this, &VoiceTyping::downloadErrorChanged);
    connect(m_models, &VoiceModels::downloaded, this, [this](const QString &id) {
        if (m_state == VoiceStates::needsModel && id == modelId()) {
            setState(VoiceStates::idle);
            start();
        }
    });
}

void VoiceTyping::connectSettings()
{
    KBoardSettings *settings = KBoardSettings::self();
    connect(settings, &KBoardSettings::voiceModelChanged, this, &VoiceTyping::onModelSettingsChanged);
    connect(settings, &KBoardSettings::voiceDeviceChanged, this, &VoiceTyping::onModelSettingsChanged);
    connect(settings, &KBoardSettings::voiceUnloadMinutesChanged, this, &VoiceTyping::scheduleUnload);
    connect(settings, &KBoardSettings::voiceEndSilenceMsChanged, this, [this]() {
        const int ms = KBoardSettings::voiceEndSilenceMs();
        QMetaObject::invokeMethod(m_capture, [capture = m_capture, ms]() { capture->setEndSilenceMs(ms); }, Qt::QueuedConnection);
    });
}

bool VoiceTyping::canStart()
{
    const QString catalogError = m_models->catalogError();
    if (!catalogError.isEmpty()) {
        setError(catalogError);
        return false;
    }
    if (!KBoardSettings::voiceEnabled()) {
        setError(QStringLiteral("Voice typing is turned off in KBoard settings"));
        return false;
    }
    const ModelEntry *entry = m_models->current();
    if (!entry) {
        setError(QStringLiteral("The voice model \"%1\" is not in the model list").arg(KBoardSettings::voiceModel()));
        return false;
    }
    if (!m_models->isReady(*entry)) {
        setState(VoiceStates::needsModel);
        return false;
    }
    return true;
}

void VoiceTyping::start()
{
    if (m_session.active) {
        resume();
        return;
    }
    if (!canStart()) {
        return;
    }
    beginSession();
}

void VoiceTyping::beginSession()
{
    const ModelEntry *entry = m_models->current();
    m_unloadTimer.stop();
    m_session = Session {m_session.id + 1, true, false, false, 0, 0, 0};
    m_errorString.clear();
    Q_EMIT errorStringChanged();
    setPaused(false);
    m_writer.reset();
    Q_EMIT committedTextChanged();
    setPartialText(QString());

    m_engine->load(entry->engine, m_models->pathFor(*entry), KBoardSettings::voiceDevice());
    setState(m_engine->isReady() ? VoiceStates::listening : VoiceStates::loading);

    const CaptureOptions options {
        m_session.id, m_models->vadPath(), KBoardSettings::voiceMicrophone(), KBoardSettings::voiceEndSilenceMs()};
    QMetaObject::invokeMethod(m_capture, [capture = m_capture, options]() { capture->start(options); }, Qt::QueuedConnection);
}

void VoiceTyping::stop()
{
    if (!m_session.active || m_session.finishing) {
        return;
    }
    m_session.finishing = true;
    QMetaObject::invokeMethod(m_capture, &AudioCapture::finish, Qt::QueuedConnection);
    updateActiveState();
}

void VoiceTyping::cancel()
{
    if (m_session.active) {
        abortSession();
        endSession();
    } else if (m_state == VoiceStates::error || m_state == VoiceStates::needsModel) {
        setState(VoiceStates::idle);
    }
}

void VoiceTyping::abortSession()
{
    ++m_session.id;
    m_engine->dropPendingPartials();
    QMetaObject::invokeMethod(m_capture, &AudioCapture::cancel, Qt::QueuedConnection);
    InputContext::instance()->clearPreedit();
}

void VoiceTyping::pause()
{
    if (!m_session.active || m_session.finishing || m_paused) {
        return;
    }
    setPaused(true);
    setLevel(0.0);
    QMetaObject::invokeMethod(m_capture, &AudioCapture::pause, Qt::QueuedConnection);
}

void VoiceTyping::resume()
{
    if (!m_session.active || m_session.finishing || !m_paused) {
        return;
    }
    setPaused(false);
    QMetaObject::invokeMethod(m_capture, &AudioCapture::resume, Qt::QueuedConnection);
}

void VoiceTyping::togglePause()
{
    m_paused ? resume() : pause();
}

void VoiceTyping::onUtterance(quint64 utterance, const QList<float> &samples, bool endOfSpeech)
{
    m_session.finalizedUtterance = qMax(m_session.finalizedUtterance, utterance);
    ++m_session.pendingFinals;
    m_engine->transcribe(DecodeRequest {m_session.id, utterance, true, samples, KBoardSettings::voiceLanguage()});
    if (endOfSpeech && KBoardSettings::voiceAutoStop() && !m_session.finishing) {
        m_session.finishing = true;
        m_session.capturing = false;
        QMetaObject::invokeMethod(m_capture, &AudioCapture::cancel, Qt::QueuedConnection);
        updateActiveState();
    }
}

void VoiceTyping::onDecoded(const DecodeRequest &request, const SpeechEngine::Result &result)
{
    if (request.session != m_session.id || !m_session.active) {
        return;
    }
    m_session.pendingFinals -= request.final ? 1 : 0;
    if (!result.ok) {
        failSession(result.error);
        return;
    }
    if (result.audioSeconds > 0.0) {
        m_realTimeFactor = result.decodeSeconds / result.audioSeconds;
        Q_EMIT realTimeFactorChanged();
    }
    if (!request.final) {
        if (request.utterance > m_session.finalizedUtterance) {
            setPartialText(result.text);
            m_writer.preview(result.text);
        }
        return;
    }
    const QString written = m_writer.write(result.text, KBoardSettings::voiceCommands());
    Q_EMIT committedTextChanged();
    if (!written.isEmpty()) {
        Q_EMIT transcriptCommitted(written);
    }
    if (request.utterance >= m_session.liveUtterance) {
        setPartialText(QString());
    }
    finishIfDone();
}

void VoiceTyping::finishIfDone()
{
    if (m_session.active && m_session.finishing && m_session.pendingFinals == 0 && !m_session.capturing) {
        endSession();
    }
}

void VoiceTyping::endSession()
{
    closeSession();
    setState(VoiceStates::idle);
}

void VoiceTyping::closeSession()
{
    m_session.active = false;
    m_session.finishing = false;
    m_session.capturing = false;
    setPaused(false);
    setPartialText(QString());
    setLevel(0.0);
    scheduleUnload();
}

void VoiceTyping::failSession(const QString &error)
{
    if (m_session.active) {
        abortSession();
        closeSession();
    }
    setError(error);
}

void VoiceTyping::updateActiveState()
{
    if (!m_session.active) {
        return;
    }
    if (m_session.finishing) {
        setState(VoiceStates::processing);
    } else if (!m_engine->isReady()) {
        setState(VoiceStates::loading);
    } else if (m_session.capturing) {
        setState(VoiceStates::listening);
    }
}
