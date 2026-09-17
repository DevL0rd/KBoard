#pragma once

#include "voicebackends.h"

#include <QList>
#include <QString>

struct parakeet_context;
struct whisper_context;

class SpeechEngine
{
public:
    struct Result
    {
        bool ok = false;
        QString text;
        QString error;
        double audioSeconds = 0.0;
        double decodeSeconds = 0.0;
    };

    SpeechEngine();
    ~SpeechEngine();
    SpeechEngine(const SpeechEngine &) = delete;
    SpeechEngine &operator=(const SpeechEngine &) = delete;

    bool load(const QString &engine, const QString &modelPath, const QString &deviceSetting, QString *error);
    void unload();
    bool isLoaded() const;
    VoiceDevice device() const;

    Result transcribe(const QList<float> &samples, const QString &language);

    static void installLogHandler();
    static int threadCount();

private:
    VoiceDevice m_device;
    parakeet_context *m_parakeet = nullptr;
    whisper_context *m_whisper = nullptr;
};
