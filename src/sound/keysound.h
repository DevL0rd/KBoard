#pragma once

#include "soundmixer.h"
#include "soundpack.h"

#include <QObject>
#include <QQmlEngine>
#include <QRandomGenerator>
#include <QThread>
#include <QTimer>
#include <QVariantList>

#include <memory>

class AudioOutput;
class Haptics;

class KeySound : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QVariantList packs READ packs NOTIFY packsChanged)
    Q_PROPERTY(QString currentPack READ currentPack NOTIFY currentPackChanged)
    Q_PROPERTY(double latencyMs READ latencyMs NOTIFY latencyChanged)
    Q_PROPERTY(int bufferFrames READ bufferFrames NOTIFY latencyChanged)
    Q_PROPERTY(bool active READ isActive WRITE setActive NOTIFY activeChanged)
    Q_PROPERTY(bool deviceOpen READ deviceOpen NOTIFY deviceChanged)
    Q_PROPERTY(QString deviceName READ deviceName NOTIFY deviceChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorStringChanged)
    Q_PROPERTY(bool hapticsAvailable READ hapticsAvailable NOTIFY hapticsAvailableChanged)

public:
    enum class Output {
        Device,
        Offline,
    };

    struct Variation {
        float pitch = 1.0f;
        float gain = 1.0f;
    };

    static constexpr float PitchRange = 0.03f;
    static constexpr float GainRange = 0.15f;
    static constexpr int RequestedBufferFrames = 256;
    static constexpr int IdleSuspendMs = 20000;
    static constexpr int InactiveCloseMs = 1500;

    explicit KeySound(QObject *parent = nullptr);
    KeySound(Output output, const QString &soundsDirectory, QObject *parent = nullptr);
    ~KeySound() override;

    static KeySound *create(QQmlEngine *engine, QJSEngine *scriptEngine);

    Q_INVOKABLE void play(const QString &kind);
    Q_INVOKABLE void preview(const QString &packId);
    Q_INVOKABLE void stopPreview();

    QVariantList packs() const;
    QString currentPack() const;
    double latencyMs() const;
    int bufferFrames() const;
    bool isActive() const;
    void setActive(bool active);
    bool deviceOpen() const;
    QString deviceName() const;
    QString errorString() const;
    bool hapticsAvailable() const;

    static Variation variation(bool enabled, QRandomGenerator &generator);
    static float volumeGain(double volume);

    const SoundPack *pack(const QString &id) const;
    bool playKind(SoundKind kind, const SoundPack *pack, bool variation);
    std::vector<float> renderOffline(int frames, int channels = 1);
    SoundMixer &mixer();

Q_SIGNALS:
    void packsChanged();
    void currentPackChanged();
    void latencyChanged();
    void activeChanged();
    void deviceChanged();
    void errorStringChanged();
    void hapticsAvailableChanged();
    void played(const QString &kind);

private:
    void loadPacks();
    void applyPack();
    void applyVolume();
    void ensureOpen();
    void updateLatency();
    void refreshErrorString();

    Output m_output;
    QString m_soundsDirectory;
    QList<std::shared_ptr<const SoundPack>> m_packs;
    QStringList m_loadErrors;
    const SoundPack *m_current = nullptr;
    QString m_packError;
    QString m_deviceError;
    QString m_errorString;

    SoundMixer m_mixer;
    QThread m_audioThread;
    AudioOutput *m_audio = nullptr;
    QTimer m_idleTimer;
    bool m_active = false;
    bool m_deviceOpen = false;
    bool m_opening = false;
    QString m_deviceName;
    int m_deviceRate = 0;
    int m_bufferFrames = 0;
    double m_latencyMs = 0.0;

    Haptics *m_haptics = nullptr;
    QRandomGenerator m_random;
    std::array<int, static_cast<size_t>(SoundKind::Count)> m_lastVariant{};
    int m_previewGeneration = 0;
};
