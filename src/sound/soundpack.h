#pragma once

#include <QList>
#include <QString>

#include <array>
#include <memory>
#include <vector>

enum class SoundKind
{
    Key,
    Space,
    Backspace,
    Enter,
    Modifier,
    Popup,
    Open,
    Close,
    Count,
};

struct SoundSample
{
    std::vector<float> frames;
    int sampleRate = 0;
    QString fileName;
};

struct SoundPack
{
    QString id;
    QString name;
    QString description;
    int order = 0;
    std::array<std::vector<SoundSample>, static_cast<size_t>(SoundKind::Count)> samples;

    const std::vector<SoundSample> &variants(SoundKind kind) const { return samples[static_cast<size_t>(kind)]; }
};

class SoundPackLoader
{
public:
    static constexpr int RequiredSampleRate = 48000;
    static constexpr int MaxKeyVariants = 8;

    static bool kindFromString(const QString &name, SoundKind *kind);
    static QStringList requiredFiles();

    static bool readWav(const QString &path, SoundSample *sample, QString *error);
    static std::shared_ptr<const SoundPack> loadPack(const QString &directory, QString *error);
    static QList<std::shared_ptr<const SoundPack>> loadAll(const QString &root, QStringList *errors);
};
