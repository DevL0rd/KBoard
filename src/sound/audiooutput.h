#pragma once

#include <QObject>
#include <QString>

#include <memory>

class QAudioSink;
class QMediaDevices;
class SoundMixer;

class AudioOutput : public QObject
{
    Q_OBJECT

public:
    explicit AudioOutput(SoundMixer *mixer, int bufferFrames);
    ~AudioOutput() override;

public Q_SLOTS:
    void open();
    void suspend();
    void close();

Q_SIGNALS:
    void opened(const QString &deviceName, int sampleRate, int channels, int bufferFrames);
    void suspended();
    void closed();
    void failed(const QString &message);

private:
    void reopenForDeviceChange();

    SoundMixer *m_mixer;
    int m_bufferFrames;
    std::unique_ptr<QAudioSink> m_sink;
    QMediaDevices *m_devices = nullptr;
    QByteArray m_deviceId;
    QString m_deviceName;
};
