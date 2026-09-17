#include "audiooutput.h"
#include "soundmixer.h"

#include <QAudioDevice>
#include <QAudioFormat>
#include <QAudioSink>
#include <QMediaDevices>
#include <QSpan>

#include <algorithm>

AudioOutput::AudioOutput(SoundMixer *mixer, int bufferFrames)
    : m_mixer(mixer)
    , m_bufferFrames(bufferFrames)
{
}

AudioOutput::~AudioOutput()
{
    if (m_sink) {
        m_sink->stop();
    }
}

void AudioOutput::open()
{
    if (!m_devices) {
        m_devices = new QMediaDevices(this);
        connect(m_devices, &QMediaDevices::audioOutputsChanged, this, &AudioOutput::reopenForDeviceChange);
    }

    if (m_sink) {
        if (m_sink->state() == QtAudio::SuspendedState) {
            m_mixer->markStreamStart();
            m_sink->resume();
        }
        if (m_sink->state() == QtAudio::ActiveState || m_sink->state() == QtAudio::IdleState) {
            Q_EMIT opened(m_deviceName, m_sink->format().sampleRate(), m_sink->format().channelCount(), static_cast<int>(m_sink->bufferFrameCount()));
            return;
        }
        m_sink->stop();
        m_sink.reset();
    }

    const QAudioDevice device = QMediaDevices::defaultAudioOutput();
    if (device.isNull()) {
        Q_EMIT failed(QStringLiteral("No audio output device"));
        return;
    }

    QAudioFormat format;
    format.setSampleRate(48000);
    format.setChannelCount(std::clamp(device.preferredFormat().channelCount(), 1, 2));
    format.setSampleFormat(QAudioFormat::Float);
    if (!device.isFormatSupported(format)) {
        Q_EMIT failed(QStringLiteral("%1 does not accept %2 Hz float audio").arg(device.description()).arg(format.sampleRate()));
        return;
    }

    m_mixer->setOutputRate(format.sampleRate());
    const int channels = format.channelCount();
    m_sink = std::make_unique<QAudioSink>(device, format);
    m_sink->setBufferFrameCount(m_bufferFrames);
    SoundMixer *mixer = m_mixer;
    m_mixer->markStreamStart();
    m_sink->start([mixer, channels](QSpan<float> buffer) {
        mixer->render(std::span<float>(buffer.data(), static_cast<size_t>(buffer.size())), channels);
    });

    if (m_sink->error() != QtAudio::NoError) {
        const auto code = m_sink->error();
        m_sink.reset();
        Q_EMIT failed(QStringLiteral("Could not open %1 (QtAudio error %2)").arg(device.description()).arg(static_cast<int>(code)));
        return;
    }

    m_deviceId = device.id();
    m_deviceName = device.description();
    Q_EMIT opened(device.description(), format.sampleRate(), channels, static_cast<int>(m_sink->bufferFrameCount()));
}

void AudioOutput::suspend()
{
    if (m_sink && m_sink->state() == QtAudio::ActiveState) {
        m_sink->suspend();
        Q_EMIT suspended();
    }
}

void AudioOutput::close()
{
    if (!m_sink) {
        return;
    }
    m_sink->stop();
    m_sink.reset();
    m_deviceId.clear();
    Q_EMIT closed();
}

void AudioOutput::reopenForDeviceChange()
{
    if (!m_sink || QMediaDevices::defaultAudioOutput().id() == m_deviceId) {
        return;
    }
    const bool wasSuspended = m_sink->state() == QtAudio::SuspendedState;
    m_sink->stop();
    m_sink.reset();
    open();
    if (wasSuspended) {
        suspend();
    }
}
