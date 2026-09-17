#include "audiocapture.h"

#include "voicelogging.h"

#include <QAudioDevice>
#include <QAudioFormat>
#include <QAudioSource>
#include <QMediaDevices>

#include <cmath>

AudioCapture::AudioCapture(QObject *parent)
    : QObject(parent)
{ }

AudioCapture::~AudioCapture()
{
    stopSource();
}

QVariantList AudioCapture::microphones()
{
    QVariantList list;
    const QAudioDevice preferred = QMediaDevices::defaultAudioInput();
    const QList<QAudioDevice> inputs = QMediaDevices::audioInputs();
    for (const QAudioDevice &device : inputs) {
        list.append(QVariantMap {
            {QStringLiteral("id"), QString::fromUtf8(device.id())},
            {QStringLiteral("description"), device.description()},
            {QStringLiteral("isDefault"), device.id() == preferred.id()},
        });
    }
    return list;
}

QAudioDevice AudioCapture::findMicrophone(const QString &id, QString *error)
{
    if (id.isEmpty()) {
        const QAudioDevice device = QMediaDevices::defaultAudioInput();
        if (device.isNull() && error) {
            *error = QStringLiteral("No microphone was found");
        }
        return device;
    }
    const QList<QAudioDevice> inputs = QMediaDevices::audioInputs();
    for (const QAudioDevice &device : inputs) {
        if (QString::fromUtf8(device.id()) == id) {
            return device;
        }
    }
    if (error) {
        *error = QStringLiteral("The chosen microphone \"%1\" is not connected").arg(id);
    }
    return {};
}

void AudioCapture::start(const CaptureOptions &options)
{
    stopSource();
    m_session = options.session;
    QString error;
    if (!m_vad.load(options.vadModelPath, &error)) {
        Q_EMIT failed(m_session, error);
        return;
    }
    m_segmenter.reset();
    m_segmenter.setEndSilenceMs(options.endSilenceMs);
    m_meter.reset();
    m_emittedLevel = -1.0;
    m_paused = false;

    const QAudioDevice device = findMicrophone(options.microphoneId, &error);
    if (device.isNull() || !openSource(device, &error)) {
        stopSource();
        Q_EMIT failed(m_session, error);
        return;
    }
    qCInfo(KBOARD_VOICE) << "Listening on" << device.description();
    Q_EMIT started(m_session, device.description());
}

bool AudioCapture::openSource(const QAudioDevice &device, QString *error)
{
    QAudioFormat format;
    format.setSampleRate(16000);
    format.setChannelCount(1);
    format.setSampleFormat(QAudioFormat::Float);
    if (!device.isFormatSupported(format)) {
        *error = QStringLiteral("The microphone \"%1\" cannot record 16 kHz mono audio").arg(device.description());
        return false;
    }
    m_source = new QAudioSource(device, format, this);
    m_source->setBufferSize(16000 * sizeof(float));
    connect(m_source, &QAudioSource::stateChanged, this, [this](QtAudio::State state) {
        if (!m_source || state != QtAudio::StoppedState || m_source->error() == QtAudio::NoError) {
            return;
        }
        stopSource();
        Q_EMIT failed(m_session, QStringLiteral("The microphone stopped working"));
    });
    m_io = m_source->start();
    if (!m_io || m_source->error() != QtAudio::NoError) {
        *error = QStringLiteral("The microphone \"%1\" could not be opened").arg(device.description());
        return false;
    }
    connect(m_io, &QIODevice::readyRead, this, &AudioCapture::onReadyRead);
    return true;
}

void AudioCapture::finish()
{
    if (m_io) {
        onReadyRead();
    }
    stopSource();
    const SpeechSegmenter::Output output = m_segmenter.flush();
    if (output.kind == SpeechSegmenter::Output::Ended) {
        Q_EMIT utteranceAudio(m_session, output.utterance, output.samples, false);
    }
    Q_EMIT levelChanged(m_session, 0.0);
    Q_EMIT finished(m_session);
}

void AudioCapture::cancel()
{
    stopSource();
    m_segmenter.reset();
    Q_EMIT levelChanged(m_session, 0.0);
}

void AudioCapture::pause()
{
    if (!m_source || m_paused) {
        return;
    }
    onReadyRead();
    m_source->suspend();
    m_paused = true;
    const SpeechSegmenter::Output output = m_segmenter.flush();
    if (output.kind == SpeechSegmenter::Output::Ended) {
        Q_EMIT utteranceAudio(m_session, output.utterance, output.samples, false);
    }
    m_meter.reset();
    Q_EMIT levelChanged(m_session, 0.0);
}

void AudioCapture::resume()
{
    if (!m_source || !m_paused) {
        return;
    }
    m_paused = false;
    m_segmenter.reset();
    m_source->resume();
}

void AudioCapture::setEndSilenceMs(int ms)
{
    m_segmenter.setEndSilenceMs(ms);
}

void AudioCapture::unloadVad()
{
    if (!m_source) {
        m_vad.unload();
    }
}

void AudioCapture::onReadyRead()
{
    if (!m_io) {
        return;
    }
    const QByteArray data = m_io->readAll();
    const qsizetype count = data.size() / qsizetype(sizeof(float));
    if (count <= 0 || m_paused) {
        return;
    }
    const float *samples = reinterpret_cast<const float *>(data.constData());
    const double level = m_meter.process(samples, count);
    if (std::abs(level - m_emittedLevel) >= 0.004) {
        m_emittedLevel = level;
        Q_EMIT levelChanged(m_session, level);
    }
    const QList<SpeechSegmenter::Output> outputs = m_segmenter.feed(samples, count);
    for (const SpeechSegmenter::Output &output : outputs) {
        dispatch(output);
    }
}

void AudioCapture::stopSource()
{
    if (m_io) {
        disconnect(m_io, nullptr, this, nullptr);
    }
    m_io = nullptr;
    if (m_source) {
        QAudioSource *source = m_source;
        m_source = nullptr;
        disconnect(source, nullptr, this, nullptr);
        source->stop();
        delete source;
    }
    m_paused = false;
}

void AudioCapture::dispatch(const SpeechSegmenter::Output &output)
{
    switch (output.kind) {
    case SpeechSegmenter::Output::Started:
        Q_EMIT speechStarted(m_session, output.utterance);
        break;
    case SpeechSegmenter::Output::Partial:
        Q_EMIT partialAudio(m_session, output.utterance, output.samples);
        break;
    case SpeechSegmenter::Output::Ended:
        Q_EMIT utteranceAudio(m_session, output.utterance, output.samples, true);
        break;
    case SpeechSegmenter::Output::Discarded:
        Q_EMIT utteranceDiscarded(m_session, output.utterance);
        break;
    }
}
