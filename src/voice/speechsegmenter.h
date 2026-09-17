#pragma once

#include "speechdetector.h"

#include <QList>

#include <vector>

class SileroVad;

class SpeechSegmenter
{
public:
    struct Output
    {
        enum Kind
        {
            Started,
            Partial,
            Ended,
            Discarded,
        };
        Kind kind;
        quint64 utterance;
        QList<float> samples;
    };

    explicit SpeechSegmenter(SileroVad *vad);

    QList<Output> feed(const float *samples, qsizetype count);
    Output flush();
    void reset();
    void setEndSilenceMs(int ms);
    bool inSpeech() const;

private:
    void processWindow(const float *frame, int window, QList<Output> &outputs);
    void continueSpeech(const float *frame, int window, QList<Output> &outputs);
    void keepPreroll(const float *frame, int window);

    SileroVad *m_vad;
    SpeechDetector m_detector;
    std::vector<float> m_pending;
    QList<float> m_preroll;
    QList<float> m_utterance;
    quint64 m_utteranceId = 0;
    qsizetype m_lastPartialSize = 0;
    int m_partialSamples = 6400;
    int m_firstPartialSamples = 12000;
    int m_prerollSamples = 4800;
};
