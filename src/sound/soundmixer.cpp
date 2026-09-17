#include "soundmixer.h"
#include "soundpack.h"

#include <QMutexLocker>

#include <algorithm>
#include <chrono>
#include <cmath>

namespace
{
std::int64_t nowNs()
{
    return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

float limit(float value)
{
    constexpr float knee = 0.8f;
    const float magnitude = std::fabs(value);
    if (magnitude <= knee) {
        return value;
    }
    const float shaped = knee + (1.0f - knee) * std::tanh((magnitude - knee) / (1.0f - knee));
    return std::copysign(shaped, value);
}
}

SoundMixer::SoundMixer() = default;

bool SoundMixer::trigger(const SoundSample &sample, float gain, float pitch)
{
    if (sample.frames.empty() || sample.sampleRate <= 0) {
        return false;
    }
    QMutexLocker locker(&m_producerLock);
    const int tail = m_tail.load(std::memory_order_relaxed);
    const int next = (tail + 1) % QueueCapacity;
    if (next == m_head.load(std::memory_order_acquire)) {
        return false;
    }
    Trigger &slot = m_queue[tail];
    slot.data = sample.frames.data();
    slot.length = static_cast<qsizetype>(sample.frames.size());
    slot.step = pitch * static_cast<double>(sample.sampleRate) / m_outputRate.load(std::memory_order_relaxed);
    slot.gain = gain;
    slot.stampNs = nowNs();
    m_tail.store(next, std::memory_order_release);
    return true;
}

void SoundMixer::startVoice(const Trigger &trigger)
{
    Voice *target = nullptr;
    for (Voice &voice : m_voices) {
        if (!voice.data) {
            target = &voice;
            break;
        }
    }
    if (!target) {
        target = &*std::min_element(m_voices.begin(), m_voices.end(), [](const Voice &a, const Voice &b) { return a.serial < b.serial; });
    }
    target->data = trigger.data;
    target->length = trigger.length;
    target->position = 0.0;
    target->step = trigger.step;
    target->gain = trigger.gain;
    target->serial = ++m_serial;
}

void SoundMixer::render(std::span<float> interleaved, int channels)
{
    channels = std::max(channels, 1);
    const qsizetype frames = static_cast<qsizetype>(interleaved.size()) / channels;
    m_blockMs.store(frames * 1000.0 / m_outputRate.load(std::memory_order_relaxed), std::memory_order_relaxed);

    drainQueue();
    std::fill(interleaved.begin(), interleaved.end(), 0.0f);

    int active = 0;
    for (Voice &voice : m_voices) {
        if (voice.data && mixVoice(voice, interleaved, frames, channels)) {
            ++active;
        }
    }
    applyMasterGain(interleaved, frames, channels);

    m_activeVoices.store(active, std::memory_order_relaxed);
    m_renderedBlocks.fetch_add(1, std::memory_order_relaxed);
}

void SoundMixer::drainQueue()
{
    int head = m_head.load(std::memory_order_relaxed);
    const int tail = m_tail.load(std::memory_order_acquire);
    if (head == tail) {
        return;
    }
    const std::int64_t now = nowNs();
    const std::int64_t streamStart = m_streamStartNs.load(std::memory_order_relaxed);
    double delaySum = 0.0;
    int count = 0;
    for (; head != tail; head = (head + 1) % QueueCapacity) {
        const Trigger &pending = m_queue[head];
        startVoice(pending);
        if (pending.stampNs >= streamStart) {
            delaySum += (now - pending.stampNs) / 1.0e6;
            ++count;
        }
    }
    m_head.store(head, std::memory_order_release);
    if (count > 0) {
        const double previous = m_queueDelayMs.load(std::memory_order_relaxed);
        const double sample = delaySum / count;
        m_queueDelayMs.store(previous == 0.0 ? sample : previous * 0.8 + sample * 0.2, std::memory_order_relaxed);
    }
}

bool SoundMixer::mixVoice(Voice &voice, std::span<float> interleaved, qsizetype frames, int channels)
{
    for (qsizetype frame = 0; frame < frames; ++frame) {
        const auto index = static_cast<qsizetype>(voice.position);
        if (index + 1 >= voice.length) {
            voice.data = nullptr;
            return false;
        }
        const auto fraction = static_cast<float>(voice.position - static_cast<double>(index));
        const float value = voice.data[index] + (voice.data[index + 1] - voice.data[index]) * fraction;
        interleaved[frame * channels] += value * voice.gain;
        voice.position += voice.step;
    }
    return true;
}

void SoundMixer::applyMasterGain(std::span<float> interleaved, qsizetype frames, int channels)
{
    const float targetGain = m_masterGain.load(std::memory_order_relaxed);
    const float gainStep = frames > 0 ? (targetGain - m_currentGain) / static_cast<float>(frames) : 0.0f;
    float gain = m_currentGain;
    for (qsizetype frame = 0; frame < frames; ++frame) {
        gain += gainStep;
        const auto first = interleaved.begin() + frame * channels;
        std::fill(first, first + channels, limit(*first * gain));
    }
    m_currentGain = targetGain;
}

void SoundMixer::markStreamStart()
{
    m_streamStartNs.store(nowNs(), std::memory_order_relaxed);
}

void SoundMixer::setOutputRate(int rate)
{
    m_outputRate.store(rate, std::memory_order_relaxed);
}

void SoundMixer::setMasterGain(float gain)
{
    m_masterGain.store(gain, std::memory_order_relaxed);
}

int SoundMixer::activeVoices() const
{
    return m_activeVoices.load(std::memory_order_relaxed);
}

double SoundMixer::queueDelayMs() const
{
    return m_queueDelayMs.load(std::memory_order_relaxed);
}

double SoundMixer::blockMs() const
{
    return m_blockMs.load(std::memory_order_relaxed);
}

quint64 SoundMixer::renderedBlocks() const
{
    return m_renderedBlocks.load(std::memory_order_relaxed);
}
