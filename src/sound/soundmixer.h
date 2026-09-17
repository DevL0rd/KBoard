#pragma once

#include <QMutex>

#include <array>
#include <atomic>
#include <cstdint>
#include <span>

struct SoundSample;

class SoundMixer
{
public:
    static constexpr int MaxVoices = 32;
    static constexpr int QueueCapacity = 128;

    SoundMixer();

    bool trigger(const SoundSample &sample, float gain, float pitch);
    void render(std::span<float> interleaved, int channels);

    void markStreamStart();
    void setOutputRate(int rate);
    void setMasterGain(float gain);

    int activeVoices() const;
    double queueDelayMs() const;
    double blockMs() const;
    quint64 renderedBlocks() const;

private:
    struct Trigger
    {
        const float *data = nullptr;
        qsizetype length = 0;
        double step = 1.0;
        float gain = 1.0f;
        std::int64_t stampNs = 0;
    };

    struct Voice
    {
        const float *data = nullptr;
        qsizetype length = 0;
        double position = 0.0;
        double step = 1.0;
        float gain = 0.0f;
        std::uint64_t serial = 0;
    };

    void startVoice(const Trigger &trigger);
    void drainQueue();
    static bool mixVoice(Voice &voice, std::span<float> interleaved, qsizetype frames, int channels);
    void applyMasterGain(std::span<float> interleaved, qsizetype frames, int channels);

    QMutex m_producerLock;
    std::array<Trigger, QueueCapacity> m_queue;
    std::atomic<int> m_head {0};
    std::atomic<int> m_tail {0};

    std::array<Voice, MaxVoices> m_voices;
    std::uint64_t m_serial = 0;
    float m_currentGain = 0.0f;

    std::atomic<std::int64_t> m_streamStartNs {0};
    std::atomic<int> m_outputRate {48000};
    std::atomic<float> m_masterGain {1.0f};
    std::atomic<int> m_activeVoices {0};
    std::atomic<double> m_queueDelayMs {0.0};
    std::atomic<double> m_blockMs {0.0};
    std::atomic<quint64> m_renderedBlocks {0};
};
