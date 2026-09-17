#pragma once

#include <QtGlobal>

struct SpeechDetectorConfig
{
    int sampleRate = 16000;
    int windowSamples = 512;
    float startThreshold = 0.5f;
    float endThreshold = 0.35f;
    int minSpeechMs = 200;
    int endSilenceMs = 900;
    int maxUtteranceMs = 25000;
};

class SpeechDetector
{
public:
    using Config = SpeechDetectorConfig;

    enum class Event
    {
        None,
        Started,
        Ended,
        Discarded,
    };

    explicit SpeechDetector(const Config &config = Config());

    Event feed(float probability);
    void reset();
    void setEndSilenceMs(int ms);

    bool inSpeech() const;

private:
    int windowsFor(int ms) const;

    Config m_config;
    bool m_inSpeech = false;
    int m_speechWindows = 0;
    int m_silenceWindows = 0;
    int m_utteranceWindows = 0;
};

class LevelMeter
{
public:
    double process(const float *samples, qsizetype count);
    void reset();

private:
    double m_level = 0.0;
};
