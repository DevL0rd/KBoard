#pragma once

#include "silerovad.h"
#include "speechdetector.h"
#include "speechsegmenter.h"

#include <QList>
#include <QObject>
#include <QPointer>
#include <QVariantList>

class QAudioDevice;
class QAudioSource;
class QIODevice;

struct CaptureOptions
{
    quint64 session = 0;
    QString vadModelPath;
    QString microphoneId;
    int endSilenceMs = 900;
};

class AudioCapture : public QObject
{
    Q_OBJECT

public:
    explicit AudioCapture(QObject *parent = nullptr);
    ~AudioCapture() override;

    static QVariantList microphones();
    static QAudioDevice findMicrophone(const QString &id, QString *error);

    void start(const CaptureOptions &options);
    void finish();
    void cancel();
    void pause();
    void resume();
    void setEndSilenceMs(int ms);
    void unloadVad();

Q_SIGNALS:
    void started(quint64 session, const QString &microphone);
    void failed(quint64 session, const QString &error);
    void levelChanged(quint64 session, double level);
    void speechStarted(quint64 session, quint64 utterance);
    void partialAudio(quint64 session, quint64 utterance, const QList<float> &samples);
    void utteranceAudio(quint64 session, quint64 utterance, const QList<float> &samples, bool endOfSpeech);
    void utteranceDiscarded(quint64 session, quint64 utterance);
    void finished(quint64 session);

private:
    bool openSource(const QAudioDevice &device, QString *error);
    void onReadyRead();
    void stopSource();
    void dispatch(const SpeechSegmenter::Output &output);

    SileroVad m_vad;
    SpeechSegmenter m_segmenter {&m_vad};
    LevelMeter m_meter;
    QPointer<QAudioSource> m_source;
    QPointer<QIODevice> m_io;
    double m_emittedLevel = -1.0;
    bool m_paused = false;
    quint64 m_session = 0;
};
