#pragma once

#include <QString>

struct whisper_vad_context;

class SileroVad
{
public:
    SileroVad() = default;
    ~SileroVad();
    SileroVad(const SileroVad &) = delete;
    SileroVad &operator=(const SileroVad &) = delete;

    bool load(const QString &path, QString *error);
    void unload();
    float probability(const float *window, int count);
    void resetState();
    int windowSamples() const;

private:
    whisper_vad_context *m_context = nullptr;
    QString m_path;
};
