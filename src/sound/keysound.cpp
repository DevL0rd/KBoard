#include "keysound.h"
#include "audiooutput.h"
#include "haptics.h"

#include "kboardpaths.h"
#include "kboardsettings.h"

#include <QLoggingCategory>

namespace
{
Q_LOGGING_CATEGORY(lcSound, "kboard.sound")

struct PreviewStep
{
    int atMs;
    SoundKind kind;
};

constexpr std::array previewSteps = {
    PreviewStep {0, SoundKind::Key},
    PreviewStep {125, SoundKind::Key},
    PreviewStep {235, SoundKind::Key},
    PreviewStep {380, SoundKind::Space},
    PreviewStep {520, SoundKind::Key},
    PreviewStep {640, SoundKind::Key},
    PreviewStep {800, SoundKind::Backspace},
    PreviewStep {980, SoundKind::Enter},
};

bool isPress(SoundKind kind)
{
    return kind != SoundKind::Open && kind != SoundKind::Close;
}
}

KeySound::KeySound(QObject *parent)
    : KeySound(Output::Device, KBoardPaths::dataFile(QStringLiteral("sounds")), parent)
{ }

KeySound::KeySound(Output output, const QString &soundsDirectory, QObject *parent)
    : QObject(parent)
    , m_output(output)
    , m_soundsDirectory(soundsDirectory)
    , m_random(QRandomGenerator::securelySeeded())
{
    m_lastVariant.fill(-1);
    loadPacks();

    auto settings = KBoardSettings::self();
    connect(settings, &KBoardSettings::soundPackChanged, this, &KeySound::applyPack);
    connect(settings, &KBoardSettings::soundVolumeChanged, this, &KeySound::applyVolume);
    applyPack();
    applyVolume();

    m_haptics = new Haptics(this);
    connect(m_haptics, &Haptics::availableChanged, this, &KeySound::hapticsAvailableChanged);

    if (m_output == Output::Offline) {
        return;
    }

    m_idleTimer.setSingleShot(true);
    connect(&m_idleTimer, &QTimer::timeout, this, [this] {
        if (!m_audio) {
            return;
        }
        const char *slot = m_active ? "suspend" : "close";
        QMetaObject::invokeMethod(m_audio, slot, Qt::QueuedConnection);
    });

    m_audio = new AudioOutput(&m_mixer, RequestedBufferFrames);
    m_audio->moveToThread(&m_audioThread);
    connect(m_audio, &AudioOutput::opened, this, [this](const QString &name, int rate, int, int frames) {
        m_opening = false;
        m_deviceOpen = true;
        m_deviceName = name;
        m_deviceRate = rate;
        m_bufferFrames = frames;
        m_deviceError.clear();
        refreshErrorString();
        Q_EMIT deviceChanged();
        updateLatency();
    });
    connect(m_audio, &AudioOutput::suspended, this, [this] {
        m_deviceOpen = false;
        Q_EMIT deviceChanged();
    });
    connect(m_audio, &AudioOutput::closed, this, [this] {
        m_deviceOpen = false;
        Q_EMIT deviceChanged();
    });
    connect(m_audio, &AudioOutput::failed, this, [this](const QString &message) {
        m_opening = false;
        m_deviceOpen = false;
        m_deviceError = message;
        qCWarning(lcSound) << message;
        refreshErrorString();
        Q_EMIT deviceChanged();
    });
    m_audioThread.setObjectName(QStringLiteral("KBoardAudio"));
    m_audioThread.start(QThread::TimeCriticalPriority);
}

KeySound::~KeySound()
{
    if (m_audio) {
        AudioOutput *audio = m_audio;
        m_audio = nullptr;
        QMetaObject::invokeMethod(audio, [audio] { delete audio; }, Qt::BlockingQueuedConnection);
    }
    if (m_audioThread.isRunning()) {
        m_audioThread.quit();
        m_audioThread.wait();
    }
}

KeySound *KeySound::create(QQmlEngine *, QJSEngine *)
{
    static KeySound *instance = new KeySound;
    QJSEngine::setObjectOwnership(instance, QJSEngine::CppOwnership);
    return instance;
}

void KeySound::loadPacks()
{
    m_loadErrors.clear();
    m_packs = SoundPackLoader::loadAll(m_soundsDirectory, &m_loadErrors);
    for (const QString &error : std::as_const(m_loadErrors)) {
        qCWarning(lcSound) << error;
    }
    Q_EMIT packsChanged();
}

void KeySound::applyPack()
{
    const QString id = KBoardSettings::soundPack();
    const SoundPack *next = pack(id);
    m_packError = next ? QString() : QStringLiteral("Sound pack \"%1\" is not installed").arg(id);
    if (!next) {
        qCWarning(lcSound) << m_packError;
    }
    if (next != m_current) {
        m_current = next;
        m_lastVariant.fill(-1);
        Q_EMIT currentPackChanged();
    }
    refreshErrorString();
}

void KeySound::applyVolume()
{
    m_mixer.setMasterGain(volumeGain(KBoardSettings::soundVolume()));
}

float KeySound::volumeGain(double volume)
{
    const double clamped = std::clamp(volume, 0.0, 1.0);
    return static_cast<float>(clamped * clamped);
}

KeySound::Variation KeySound::variation(bool enabled, QRandomGenerator &generator)
{
    if (!enabled) {
        return {};
    }
    const auto spread = [&generator](float range) { return static_cast<float>(1.0 + (generator.generateDouble() * 2.0 - 1.0) * range); };
    Variation result;
    result.pitch = spread(PitchRange);
    result.gain = spread(GainRange);
    return result;
}

const SoundPack *KeySound::pack(const QString &id) const
{
    for (const auto &candidate : m_packs) {
        if (candidate->id == id) {
            return candidate.get();
        }
    }
    return nullptr;
}

void KeySound::play(const QString &kind)
{
    SoundKind soundKind;
    if (!SoundPackLoader::kindFromString(kind, &soundKind)) {
        qCWarning(lcSound) << "Unknown sound kind" << kind;
        return;
    }
    if (KBoardSettings::haptics() && m_haptics->available() && isPress(soundKind)) {
        m_haptics->trigger(QStringLiteral("button-pressed"));
    }
    if (!KBoardSettings::soundEnabled() || !m_current) {
        return;
    }
    if (playKind(soundKind, m_current, KBoardSettings::soundVariation())) {
        Q_EMIT played(kind);
    }
}

bool KeySound::playKind(SoundKind kind, const SoundPack *soundPack, bool vary)
{
    const auto &variants = soundPack->variants(kind);
    if (variants.empty()) {
        return false;
    }
    int index = 0;
    if (variants.size() > 1) {
        const int last = m_lastVariant[static_cast<size_t>(kind)];
        index = static_cast<int>(m_random.bounded(static_cast<quint32>(variants.size() - (last >= 0 ? 1 : 0))));
        if (last >= 0 && index >= last) {
            ++index;
        }
    }
    m_lastVariant[static_cast<size_t>(kind)] = index;

    const Variation shaped = variation(vary, m_random);
    if (!m_mixer.trigger(variants[static_cast<size_t>(index)], shaped.gain, shaped.pitch)) {
        return false;
    }
    ensureOpen();
    updateLatency();
    return true;
}

void KeySound::preview(const QString &packId)
{
    const SoundPack *target = pack(packId);
    if (!target) {
        qCWarning(lcSound) << "Cannot preview missing sound pack" << packId;
        return;
    }
    const int generation = ++m_previewGeneration;
    for (const PreviewStep &step : previewSteps) {
        QTimer::singleShot(step.atMs, Qt::PreciseTimer, this, [this, generation, packId, kind = step.kind] {
            if (generation != m_previewGeneration) {
                return;
            }
            if (const SoundPack *current = pack(packId)) {
                playKind(kind, current, KBoardSettings::soundVariation());
            }
        });
    }
}

void KeySound::stopPreview()
{
    ++m_previewGeneration;
}

void KeySound::ensureOpen()
{
    if (!m_audio) {
        return;
    }
    m_idleTimer.start(m_active ? IdleSuspendMs : InactiveCloseMs);
    if (m_deviceOpen || m_opening) {
        return;
    }
    m_opening = true;
    QMetaObject::invokeMethod(m_audio, &AudioOutput::open, Qt::QueuedConnection);
}

void KeySound::updateLatency()
{
    if (m_mixer.renderedBlocks() == 0) {
        return;
    }
    const double latency = m_mixer.queueDelayMs() + m_mixer.blockMs();
    const int frames = m_deviceRate > 0 ? static_cast<int>(std::lround(m_mixer.blockMs() * m_deviceRate / 1000.0)) : m_bufferFrames;
    if (std::abs(latency - m_latencyMs) < 0.05 && frames == m_bufferFrames) {
        return;
    }
    m_latencyMs = latency;
    m_bufferFrames = frames;
    Q_EMIT latencyChanged();
}

void KeySound::refreshErrorString()
{
    QStringList parts = m_loadErrors;
    if (!m_packError.isEmpty()) {
        parts.append(m_packError);
    }
    if (!m_deviceError.isEmpty()) {
        parts.append(m_deviceError);
    }
    const QString combined = parts.join(QLatin1Char('\n'));
    if (combined == m_errorString) {
        return;
    }
    m_errorString = combined;
    Q_EMIT errorStringChanged();
}

QVariantList KeySound::packs() const
{
    QVariantList list;
    for (const auto &entry : m_packs) {
        list.append(QVariantMap {
            {QStringLiteral("id"), entry->id},
            {QStringLiteral("name"), entry->name},
            {QStringLiteral("description"), entry->description},
        });
    }
    return list;
}

QString KeySound::currentPack() const
{
    return m_current ? m_current->id : QString();
}

double KeySound::latencyMs() const
{
    return m_latencyMs;
}

int KeySound::bufferFrames() const
{
    return m_bufferFrames;
}

bool KeySound::isActive() const
{
    return m_active;
}

void KeySound::setActive(bool active)
{
    if (m_active == active) {
        return;
    }
    m_active = active;
    Q_EMIT activeChanged();
    if (!m_audio) {
        return;
    }
    if (active) {
        ensureOpen();
    } else {
        m_idleTimer.start(InactiveCloseMs);
    }
}

bool KeySound::deviceOpen() const
{
    return m_deviceOpen;
}

QString KeySound::deviceName() const
{
    return m_deviceName;
}

QString KeySound::errorString() const
{
    return m_errorString;
}

bool KeySound::hapticsAvailable() const
{
    return m_haptics->available();
}

std::vector<float> KeySound::renderOffline(int frames, int channels)
{
    if (m_output != Output::Offline) {
        qCWarning(lcSound) << "renderOffline needs a KeySound created with Output::Offline";
        return {};
    }
    std::vector<float> buffer(static_cast<size_t>(frames) * static_cast<size_t>(channels));
    m_mixer.render(buffer, channels);
    return buffer;
}

SoundMixer &KeySound::mixer()
{
    return m_mixer;
}
