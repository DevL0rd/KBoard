#include "speechdetector.h"

#include <algorithm>
#include <cmath>

SpeechDetector::SpeechDetector(const Config &config)
    : m_config(config)
{ }

SpeechDetector::Event SpeechDetector::feed(float probability)
{
    if (!m_inSpeech) {
        if (probability < m_config.startThreshold) {
            return Event::None;
        }
        m_inSpeech = true;
        m_speechWindows = 1;
        m_silenceWindows = 0;
        m_utteranceWindows = 1;
        return Event::Started;
    }

    ++m_utteranceWindows;
    if (probability < m_config.endThreshold) {
        ++m_silenceWindows;
    } else {
        m_silenceWindows = 0;
        ++m_speechWindows;
    }

    const bool silentLongEnough = m_silenceWindows >= windowsFor(m_config.endSilenceMs);
    const bool tooLong = m_utteranceWindows >= windowsFor(m_config.maxUtteranceMs);
    if (!silentLongEnough && !tooLong) {
        return Event::None;
    }
    const bool enoughSpeech = m_speechWindows >= windowsFor(m_config.minSpeechMs);
    m_inSpeech = false;
    m_speechWindows = 0;
    m_silenceWindows = 0;
    return enoughSpeech ? Event::Ended : Event::Discarded;
}

void SpeechDetector::reset()
{
    m_inSpeech = false;
    m_speechWindows = 0;
    m_silenceWindows = 0;
    m_utteranceWindows = 0;
}

void SpeechDetector::setEndSilenceMs(int ms)
{
    m_config.endSilenceMs = ms;
}

bool SpeechDetector::inSpeech() const
{
    return m_inSpeech;
}

int SpeechDetector::windowsFor(int ms) const
{
    const double windowMs = 1000.0 * m_config.windowSamples / m_config.sampleRate;
    return qMax(1, int(std::ceil(ms / windowMs)));
}

double LevelMeter::process(const float *samples, qsizetype count)
{
    if (count <= 0) {
        return m_level;
    }
    double sum = 0.0;
    for (qsizetype i = 0; i < count; ++i) {
        sum += double(samples[i]) * double(samples[i]);
    }
    const double rms = std::sqrt(sum / double(count));
    const double db = 20.0 * std::log10(std::max(rms, 1e-7));
    const double target = std::clamp((db + 60.0) / 50.0, 0.0, 1.0);
    const double coefficient = target > m_level ? 0.6 : 0.18;
    m_level += (target - m_level) * coefficient;
    return m_level;
}

void LevelMeter::reset()
{
    m_level = 0.0;
}
