#include "speechdetector.h"
#include "audiocapture.h"
#include "voicetestutils.h"

#include <QTest>

#include <memory>

class SpeechDetectorTest : public QObject
{
    Q_OBJECT

private:
    using Event = SpeechDetector::Event;

    static QList<Event> run(SpeechDetector &detector, const QList<float> &probabilities)
    {
        QList<Event> events;
        for (float probability : probabilities) {
            const Event event = detector.feed(probability);
            if (event != Event::None) {
                events.append(event);
            }
        }
        return events;
    }

    static QList<float> repeated(float value, int windows) { return QList<float>(windows, value); }

    static QList<SpeechSegmenter::Output::Kind> segment(SpeechSegmenter &segmenter, const QList<float> &audio, QList<float> *ended)
    {
        QList<SpeechSegmenter::Output::Kind> kinds;
        for (qsizetype offset = 0; offset < audio.size(); offset += 320) {
            const qsizetype count = qMin<qsizetype>(320, audio.size() - offset);
            const QList<SpeechSegmenter::Output> outputs = segmenter.feed(audio.constData() + offset, count);
            for (const SpeechSegmenter::Output &output : outputs) {
                if (output.kind == SpeechSegmenter::Output::Partial && !kinds.isEmpty() && kinds.last() == output.kind) {
                    continue;
                }
                kinds.append(output.kind);
                if (output.kind == SpeechSegmenter::Output::Ended && ended) {
                    *ended = output.samples;
                }
            }
        }
        return kinds;
    }

    static std::unique_ptr<SileroVad> loadedVad()
    {
        const QString path = VoiceTest::installedModelPath(QStringLiteral("silero-v6.2.0"));
        auto vad = std::make_unique<SileroVad>();
        QString error;
        if (path.isEmpty() || !vad->load(path, &error)) {
            return {};
        }
        return vad;
    }

    static QList<float> foxSpeech() { return VoiceTest::readWav16kMono(VoiceTest::dataPath(QStringLiteral("quick-brown-fox.wav"))); }

private Q_SLOTS:
    void endsAfterSilence()
    {
        SpeechDetector detector;
        QList<float> probabilities = repeated(0.05f, 20) + repeated(0.9f, 30) + repeated(0.1f, 40);
        QCOMPARE(run(detector, probabilities), (QList<Event> {Event::Started, Event::Ended}));
        QVERIFY(!detector.inSpeech());
    }

    void shortPausesKeepUtterance()
    {
        SpeechDetector detector;
        const QList<float> probabilities
            = repeated(0.9f, 20) + repeated(0.1f, 15) + repeated(0.9f, 20) + repeated(0.4f, 10) + repeated(0.8f, 5);
        QCOMPARE(run(detector, probabilities), (QList<Event> {Event::Started}));
        QVERIFY(detector.inSpeech());
    }

    void blipsAreDiscarded()
    {
        SpeechDetector detector;
        const QList<float> probabilities = repeated(0.95f, 2) + repeated(0.0f, 40);
        QCOMPARE(run(detector, probabilities), (QList<Event> {Event::Started, Event::Discarded}));
    }

    void longSpeechIsSplit()
    {
        SpeechDetector::Config config;
        config.maxUtteranceMs = 2000;
        SpeechDetector detector(config);
        QCOMPARE(run(detector, repeated(0.9f, 70)), (QList<Event> {Event::Started, Event::Ended, Event::Started}));
    }

    void endSilenceIsConfigurable()
    {
        SpeechDetector detector;
        detector.setEndSilenceMs(300);
        QCOMPARE(run(detector, repeated(0.9f, 20) + repeated(0.0f, 10)), (QList<Event> {Event::Started, Event::Ended}));
    }

    void levelMeterFollowsLoudness()
    {
        LevelMeter meter;
        const QList<float> quiet = VoiceTest::silence(0.05);
        const QList<float> loud = VoiceTest::tone(0.05, 440.0, 0.5f);
        QCOMPARE(meter.process(quiet.constData(), quiet.size()), 0.0);
        double level = 0.0;
        for (int i = 0; i < 10; ++i) {
            level = meter.process(loud.constData(), loud.size());
        }
        QVERIFY(level > 0.8 && level <= 1.0);
        for (int i = 0; i < 40; ++i) {
            level = meter.process(quiet.constData(), quiet.size());
        }
        QVERIFY(level < 0.05);
    }

    void sileroFindsSpeechInSyntheticAudio()
    {
        const std::unique_ptr<SileroVad> vad = loadedVad();
        if (!vad) {
            QSKIP("Silero VAD model is not downloaded");
        }
        SpeechSegmenter segmenter(vad.get());
        const QList<float> speech = foxSpeech();
        QVERIFY(speech.size() > 16000);
        const QList<float> audio = VoiceTest::silence(1.0) + speech + VoiceTest::silence(1.5);
        QList<float> ended;
        using Kind = SpeechSegmenter::Output::Kind;
        QCOMPARE(segment(segmenter, audio, &ended), (QList<Kind> {Kind::Started, Kind::Partial, Kind::Ended}));
        QVERIFY(ended.size() >= speech.size() * 0.8);
        QVERIFY(ended.size() <= speech.size() + 16000 * 1.5);
    }

    void sileroIgnoresNoiseAndTones()
    {
        const std::unique_ptr<SileroVad> vad = loadedVad();
        if (!vad) {
            QSKIP("Silero VAD model is not downloaded");
        }
        SpeechSegmenter segmenter(vad.get());
        const QList<float> audio
            = VoiceTest::silence(0.5) + VoiceTest::noise(2.0, 0.05f) + VoiceTest::tone(2.0, 1000.0, 0.3f) + VoiceTest::silence(1.0);
        QCOMPARE(segment(segmenter, audio, nullptr), QList<SpeechSegmenter::Output::Kind>());
    }

    void flushReturnsSpeechInProgress()
    {
        const std::unique_ptr<SileroVad> vad = loadedVad();
        if (!vad) {
            QSKIP("Silero VAD model is not downloaded");
        }
        SpeechSegmenter segmenter(vad.get());
        const QList<float> speech = foxSpeech();
        segmenter.feed(speech.constData(), speech.size() / 2);
        QVERIFY(segmenter.inSpeech());
        const SpeechSegmenter::Output output = segmenter.flush();
        QCOMPARE(output.kind, SpeechSegmenter::Output::Ended);
        QVERIFY(output.samples.size() > 8000);
        QVERIFY(!segmenter.inSpeech());
    }

    void missingVadModelIsAnError()
    {
        SileroVad vad;
        QString error;
        QVERIFY(!vad.load(QStringLiteral("/nonexistent/vad.bin"), &error));
        QVERIFY(error.contains(QStringLiteral("missing")));
    }
};

QTEST_GUILESS_MAIN(SpeechDetectorTest)
#include "speechdetectortest.moc"
