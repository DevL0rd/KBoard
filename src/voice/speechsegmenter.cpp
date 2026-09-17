#include "speechsegmenter.h"

#include "silerovad.h"

#include <algorithm>

namespace
{
void appendSamples(QList<float> &list, const float *samples, qsizetype count)
{
    const qsizetype start = list.size();
    list.resize(start + count);
    std::copy_n(samples, count, list.begin() + start);
}
}

SpeechSegmenter::SpeechSegmenter(SileroVad *vad)
    : m_vad(vad)
{ }

QList<SpeechSegmenter::Output> SpeechSegmenter::feed(const float *samples, qsizetype count)
{
    QList<Output> outputs;
    const int window = m_vad->windowSamples();
    m_pending.insert(m_pending.end(), samples, samples + count);
    size_t offset = 0;
    while (m_pending.size() - offset >= size_t(window)) {
        processWindow(m_pending.data() + offset, window, outputs);
        offset += window;
    }
    m_pending.erase(m_pending.begin(), m_pending.begin() + qsizetype(offset));
    return outputs;
}

void SpeechSegmenter::processWindow(const float *frame, int window, QList<Output> &outputs)
{
    const bool wasInSpeech = m_detector.inSpeech();
    switch (m_detector.feed(m_vad->probability(frame, window))) {
    case SpeechDetector::Event::Started:
        ++m_utteranceId;
        m_utterance = m_preroll;
        appendSamples(m_utterance, frame, window);
        m_preroll.clear();
        m_lastPartialSize = 0;
        outputs.append(Output {Output::Started, m_utteranceId, {}});
        break;
    case SpeechDetector::Event::Ended:
        appendSamples(m_utterance, frame, window);
        outputs.append(Output {Output::Ended, m_utteranceId, m_utterance});
        m_utterance.clear();
        m_vad->resetState();
        break;
    case SpeechDetector::Event::Discarded:
        outputs.append(Output {Output::Discarded, m_utteranceId, {}});
        m_utterance.clear();
        m_vad->resetState();
        break;
    case SpeechDetector::Event::None:
        if (wasInSpeech) {
            continueSpeech(frame, window, outputs);
        } else {
            keepPreroll(frame, window);
        }
        break;
    }
}

void SpeechSegmenter::continueSpeech(const float *frame, int window, QList<Output> &outputs)
{
    appendSamples(m_utterance, frame, window);
    if (m_utterance.size() >= m_firstPartialSamples && m_utterance.size() - m_lastPartialSize >= m_partialSamples) {
        m_lastPartialSize = m_utterance.size();
        outputs.append(Output {Output::Partial, m_utteranceId, m_utterance});
    }
}

void SpeechSegmenter::keepPreroll(const float *frame, int window)
{
    appendSamples(m_preroll, frame, window);
    if (m_preroll.size() > m_prerollSamples) {
        m_preroll.remove(0, m_preroll.size() - m_prerollSamples);
    }
}

SpeechSegmenter::Output SpeechSegmenter::flush()
{
    Output output {Output::Discarded, m_utteranceId, {}};
    if (m_detector.inSpeech()) {
        appendSamples(m_utterance, m_pending.data(), qsizetype(m_pending.size()));
        output.kind = Output::Ended;
        output.samples = m_utterance;
    }
    reset();
    return output;
}

void SpeechSegmenter::reset()
{
    m_detector.reset();
    m_pending.clear();
    m_preroll.clear();
    m_utterance.clear();
    m_lastPartialSize = 0;
    m_vad->resetState();
}

void SpeechSegmenter::setEndSilenceMs(int ms)
{
    m_detector.setEndSilenceMs(ms);
}

bool SpeechSegmenter::inSpeech() const
{
    return m_detector.inSpeech();
}
