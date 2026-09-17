#include "speechengine.h"
#include "voicebackends.h"
#include "voicetestutils.h"

#include <QRegularExpression>
#include <QTest>

class TranscriptionTest : public QObject
{
    Q_OBJECT

private:
    static VoiceDevice device(const QString &name, const QString &type, const QString &backend, qint64 memory)
    {
        VoiceDevice result;
        result.name = name;
        result.type = type;
        result.backend = backend;
        result.memoryTotal = memory;
        result.description = name;
        return result;
    }

    static QString simplified(const QString &text)
    {
        static const QRegularExpression punctuation(QStringLiteral("[^a-z ]"));
        return text.toLower().remove(punctuation).simplified();
    }

    static QString modelPathOrSkip() { return VoiceTest::installedModelPath(QStringLiteral("parakeet-tdt-0.6b-v3-q4_0")); }

    void transcribeOn(const QString &engineName, const QString &modelId, const QString &deviceSetting)
    {
        const QString path = VoiceTest::installedModelPath(modelId);
        if (path.isEmpty()) {
            QSKIP("The voice model for this test is not downloaded");
        }
        SpeechEngine engine;
        QString error;
        QVERIFY2(engine.load(engineName, path, deviceSetting, &error), qPrintable(error));
        QVERIFY(engine.isLoaded());
        const QList<float> audio = VoiceTest::readWav16kMono(VoiceTest::dataPath(QStringLiteral("quick-brown-fox.wav")));
        engine.transcribe(audio, QStringLiteral("auto"));
        const SpeechEngine::Result result = engine.transcribe(audio, QStringLiteral("auto"));
        QVERIFY2(result.ok, qPrintable(result.error));
        qInfo("%s %s on %s: \"%s\" real-time factor %.4f", qPrintable(modelId), qPrintable(deviceSetting),
            qPrintable(engine.device().label()), qPrintable(result.text), result.decodeSeconds / result.audioSeconds);
        const QString heard = simplified(result.text);
        QVERIFY2(heard.startsWith(QStringLiteral("the quick brown fox")) && heard.endsWith(QStringLiteral("lazy dog")), qPrintable(heard));
    }

private Q_SLOTS:
    void autoPrefersDiscreteGpuThenBackendOrder()
    {
        const QList<VoiceDevice> devices {
            device(QStringLiteral("Vulkan1"), QStringLiteral("igpu"), QStringLiteral("Vulkan"), 1000),
            device(QStringLiteral("CPU"), QStringLiteral("cpu"), QStringLiteral("CPU"), 0),
            device(QStringLiteral("Vulkan0"), QStringLiteral("gpu"), QStringLiteral("Vulkan"), 16000),
            device(QStringLiteral("CUDA0"), QStringLiteral("gpu"), QStringLiteral("CUDA"), 16000),
        };
        QString error;
        QCOMPARE(VoiceBackends::select(devices, QStringLiteral("auto"), &error).name, QStringLiteral("CUDA0"));
        QCOMPARE(VoiceBackends::select(devices.mid(0, 3), QStringLiteral("auto"), &error).name, QStringLiteral("Vulkan0"));
        QCOMPARE(VoiceBackends::select(devices.mid(0, 2), QString(), &error).name, QStringLiteral("Vulkan1"));
        QCOMPARE(VoiceBackends::select(devices.mid(1, 1), QStringLiteral("auto"), &error).name, QStringLiteral("CPU"));
        QCOMPARE(VoiceBackends::select(devices, QStringLiteral("cpu"), &error).name, QStringLiteral("CPU"));
    }

    void unknownDeviceIsAnError()
    {
        QString error;
        const VoiceDevice chosen = VoiceBackends::select(
            {device(QStringLiteral("CPU"), QStringLiteral("cpu"), QStringLiteral("CPU"), 0)}, QStringLiteral("ROCm0"), &error);
        QVERIFY(chosen.name.isEmpty());
        QVERIFY(error.contains(QStringLiteral("ROCm0")));
        QVERIFY(error.contains(QStringLiteral("CPU")));
        QVERIFY(VoiceBackends::select({}, QStringLiteral("auto"), &error).name.isEmpty());
    }

    void labelsAreShort()
    {
        VoiceDevice gpu = device(QStringLiteral("CUDA0"), QStringLiteral("gpu"), QStringLiteral("CUDA"), 1);
        gpu.description = QStringLiteral("NVIDIA GeForce RTX 4090 Laptop GPU");
        QCOMPARE(gpu.label(), QStringLiteral("CUDA · RTX 4090 Laptop GPU"));
        QCOMPARE(VoiceBackends::shortDescription(QStringLiteral("Intel(R) Graphics (RPL-S)")), QStringLiteral("Graphics (RPL-S)"));
        QCOMPARE(device(QStringLiteral("CPU"), QStringLiteral("cpu"), QStringLiteral("CPU"), 0).label(), QStringLiteral("CPU"));
    }

    void realDevicesAreListed()
    {
        const QList<VoiceDevice> devices = VoiceBackends::devices();
        QVERIFY(std::any_of(devices.cbegin(), devices.cend(), [](const VoiceDevice &d) { return d.type == u"cpu"; }));
    }

    void transcribesOnCpu()
    {
        transcribeOn(QStringLiteral("parakeet"), QStringLiteral("parakeet-tdt-0.6b-v3-q4_0"), QStringLiteral("CPU"));
    }

    void transcribesOnBestDevice()
    {
        transcribeOn(QStringLiteral("parakeet"), QStringLiteral("parakeet-tdt-0.6b-v3-q4_0"), QStringLiteral("auto"));
    }

    void whisperTranscribes() { transcribeOn(QStringLiteral("whisper"), QStringLiteral("whisper-base-q5_1"), QStringLiteral("auto")); }

    void missingModelIsAnError()
    {
        SpeechEngine engine;
        QString error;
        QVERIFY(!engine.load(QStringLiteral("parakeet"), QStringLiteral("/nonexistent/model.bin"), QStringLiteral("auto"), &error));
        QVERIFY(error.contains(QStringLiteral("missing")));
        QVERIFY(!engine.transcribe(VoiceTest::silence(1.0), QString()).ok);
    }

    void unknownDeviceFailsLoading()
    {
        const QString path = modelPathOrSkip();
        if (path.isEmpty()) {
            QSKIP("Parakeet q4_0 model is not downloaded");
        }
        SpeechEngine engine;
        QString error;
        QVERIFY(!engine.load(QStringLiteral("parakeet"), path, QStringLiteral("NoSuchGpu0"), &error));
        QVERIFY(error.contains(QStringLiteral("NoSuchGpu0")));
        QVERIFY(!engine.isLoaded());
    }
};

QTEST_GUILESS_MAIN(TranscriptionTest)
#include "transcriptiontest.moc"
