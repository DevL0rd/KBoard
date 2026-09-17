#include "keysound.h"
#include "soundmixer.h"
#include "soundpack.h"

#include "kboardsettings.h"

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTest>

#include <algorithm>
#include <cmath>

class SoundTest : public QObject
{
    Q_OBJECT

private:
    static QString soundsDir() { return QStringLiteral(KBOARD_DATA_BUILD_DIR "/sounds"); }

    static float peak(const std::vector<float> &buffer)
    {
        float result = 0.0f;
        for (float value : buffer) {
            result = std::max(result, std::fabs(value));
        }
        return result;
    }

private Q_SLOTS:
    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);
        KBoardSettings::self()->setDefaults();
        KBoardSettings::setSoundEnabled(true);
        KBoardSettings::setSoundPack(QStringLiteral("soft"));
        KBoardSettings::setSoundVolume(1.0);
        KBoardSettings::setSoundVariation(true);
        KBoardSettings::setHaptics(false);
    }

    void packLoading()
    {
        QStringList errors;
        const auto packs = SoundPackLoader::loadAll(soundsDir(), &errors);
        QVERIFY2(errors.isEmpty(), qPrintable(errors.join(QLatin1Char('\n'))));
        QStringList ids;
        for (const auto &pack : packs) {
            ids << pack->id;
            QVERIFY(!pack->name.isEmpty());
            QVERIFY(!pack->description.isEmpty());
        }
        QCOMPARE(ids.first(), QStringLiteral("soft"));
        for (const char *expected : {"soft", "mechanical", "typewriter", "bubble", "glass"}) {
            QVERIFY2(ids.contains(QLatin1StringView(expected)), expected);
        }

        KeySound sound(KeySound::Output::Offline, soundsDir());
        QCOMPARE(sound.packs().size(), packs.size());
        const QVariantMap first = sound.packs().first().toMap();
        QCOMPARE(first.value(QStringLiteral("id")).toString(), QStringLiteral("soft"));
        QCOMPARE(first.value(QStringLiteral("name")).toString(), QStringLiteral("Soft"));
        QCOMPARE(sound.currentPack(), QStringLiteral("soft"));
        QVERIFY(sound.errorString().isEmpty());
    }

    void filesPresentAndValid_data()
    {
        QTest::addColumn<QString>("packId");
        const QStringList entries = QDir(soundsDir()).entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
        for (const QString &entry : entries) {
            QTest::newRow(qPrintable(entry)) << entry;
        }
    }

    void filesPresentAndValid()
    {
        QFETCH(QString, packId);
        const QDir dir(soundsDir() + QLatin1Char('/') + packId);
        for (const QString &file : SoundPackLoader::requiredFiles()) {
            QVERIFY2(QFile::exists(dir.filePath(file)), qPrintable(dir.filePath(file)));
        }
        for (int i = 1; i <= 4; ++i) {
            QVERIFY(QFile::exists(dir.filePath(QStringLiteral("key%1.wav").arg(i))));
        }

        QFile meta(dir.filePath(QStringLiteral("pack.json")));
        QVERIFY(meta.open(QIODevice::ReadOnly));
        const QJsonObject object = QJsonDocument::fromJson(meta.readAll()).object();
        QCOMPARE(object.value(QLatin1StringView("id")).toString(), packId);
        QVERIFY(!object.value(QLatin1StringView("name")).toString().isEmpty());
        QVERIFY(!object.value(QLatin1StringView("description")).toString().isEmpty());

        const QStringList wavs = dir.entryList({QStringLiteral("*.wav")}, QDir::Files);
        QVERIFY(wavs.size() >= 11);
        for (const QString &wav : wavs) {
            SoundSample sample;
            QString error;
            QVERIFY2(SoundPackLoader::readWav(dir.filePath(wav), &sample, &error), qPrintable(error));
            QCOMPARE(sample.sampleRate, 48000);
            const double durationMs = sample.frames.size() * 1000.0 / sample.sampleRate;
            const bool longAllowed = wav == QLatin1StringView("open.wav") || wav == QLatin1StringView("close.wav")
                || (packId == QLatin1StringView("typewriter") && wav == QLatin1StringView("enter.wav"));
            QVERIFY2(durationMs <= (longAllowed ? 600.0 : 120.0), qPrintable(QStringLiteral("%1 is %2 ms").arg(wav).arg(durationMs)));
            QVERIFY2(durationMs >= 40.0, qPrintable(wav));
            const float top = peak(sample.frames);
            QVERIFY2(top > 0.15f && top <= 1.0f, qPrintable(QStringLiteral("%1 peak %2").arg(wav).arg(top)));
            QVERIFY2(std::fabs(sample.frames.front()) < 0.01f, qPrintable(wav));
            QVERIFY2(std::fabs(sample.frames.back()) < 0.01f, qPrintable(wav));
            const size_t tail = sample.frames.size() / 20;
            float tailPeak = 0.0f;
            for (size_t i = sample.frames.size() - tail; i < sample.frames.size(); ++i) {
                tailPeak = std::max(tailPeak, std::fabs(sample.frames[i]));
            }
            QVERIFY2(tailPeak < top * 0.1f, qPrintable(QStringLiteral("%1 tail %2").arg(wav).arg(tailPeak)));
        }
    }

    void rejectsWrongFormat()
    {
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("bad.wav"));
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        QByteArray header("RIFF\x24\x00\x00\x00WAVEfmt "
                          "\x10\x00\x00\x00\x01\x00\x01\x00\x44\xac\x00\x00\x88\x58\x01\x00\x02\x00\x10\x00data\x00\x00\x00\x00",
            44);
        file.write(header);
        file.close();
        SoundSample sample;
        QString error;
        QVERIFY(!SoundPackLoader::readWav(path, &sample, &error));
        QVERIFY(error.contains(QStringLiteral("48 kHz")));
    }

    void mixerRendersPlay()
    {
        KeySound sound(KeySound::Output::Offline, soundsDir());
        QVERIFY(peak(sound.renderOffline(4800)) == 0.0f);

        QSignalSpy played(&sound, &KeySound::played);
        sound.play(QStringLiteral("key"));
        QCOMPARE(played.size(), 1);
        const auto mono = sound.renderOffline(4800);
        QVERIFY(peak(mono) > 0.05f);
        QVERIFY(peak(mono) <= 1.0f);

        sound.play(QStringLiteral("enter"));
        const auto stereo = sound.renderOffline(1024, 2);
        QVERIFY(peak(stereo) > 0.05f);
        for (size_t i = 0; i + 1 < stereo.size(); i += 2) {
            QCOMPARE(stereo[i], stereo[i + 1]);
        }
        sound.renderOffline(48000);
        QCOMPARE(sound.mixer().activeVoices(), 0);
        QVERIFY(peak(sound.renderOffline(4800)) == 0.0f);
    }

    void respectsSettings()
    {
        KeySound sound(KeySound::Output::Offline, soundsDir());

        KBoardSettings::setSoundEnabled(false);
        sound.play(QStringLiteral("key"));
        QVERIFY(peak(sound.renderOffline(4800)) == 0.0f);
        KBoardSettings::setSoundEnabled(true);

        KBoardSettings::setSoundVolume(0.0);
        sound.play(QStringLiteral("key"));
        sound.renderOffline(256);
        QVERIFY(peak(sound.renderOffline(4800)) == 0.0f);
        KBoardSettings::setSoundVolume(1.0);
        sound.renderOffline(48000);

        QSignalSpy packChanged(&sound, &KeySound::currentPackChanged);
        KBoardSettings::setSoundPack(QStringLiteral("glass"));
        QCOMPARE(packChanged.size(), 1);
        QCOMPARE(sound.currentPack(), QStringLiteral("glass"));
        sound.play(QStringLiteral("space"));
        QVERIFY(peak(sound.renderOffline(4800)) > 0.05f);

        QSignalSpy errorChanged(&sound, &KeySound::errorStringChanged);
        KBoardSettings::setSoundPack(QStringLiteral("does-not-exist"));
        QCOMPARE(errorChanged.size(), 1);
        QVERIFY(sound.errorString().contains(QStringLiteral("does-not-exist")));
        QVERIFY(sound.currentPack().isEmpty());
        sound.renderOffline(48000);
        sound.play(QStringLiteral("key"));
        QVERIFY(peak(sound.renderOffline(4800)) == 0.0f);

        KBoardSettings::setSoundPack(QStringLiteral("soft"));
        QVERIFY(sound.errorString().isEmpty());
    }

    void overlappingVoices()
    {
        KeySound sound(KeySound::Output::Offline, soundsDir());
        for (int i = 0; i < 12; ++i) {
            sound.play(QStringLiteral("key"));
        }
        const auto buffer = sound.renderOffline(512);
        QCOMPARE(sound.mixer().activeVoices(), 12);
        QVERIFY(peak(buffer) <= 1.0f);

        for (int i = 0; i < SoundMixer::MaxVoices + 8; ++i) {
            sound.play(QStringLiteral("space"));
        }
        sound.renderOffline(64);
        QCOMPARE(sound.mixer().activeVoices(), SoundMixer::MaxVoices);
    }

    void pitchResamples()
    {
        QString error;
        const auto pack = SoundPackLoader::loadPack(soundsDir() + QStringLiteral("/soft"), &error);
        QVERIFY2(pack, qPrintable(error));
        const SoundSample &sample = pack->variants(SoundKind::Key).front();
        for (float pitch : {0.97f, 1.0f, 1.03f}) {
            SoundMixer mixer;
            QVERIFY(mixer.trigger(sample, 1.0f, pitch));
            std::vector<float> block(1);
            qsizetype frames = 0;
            do {
                mixer.render(block, 1);
                ++frames;
            } while (mixer.activeVoices() > 0 && frames < 100000);
            const double expected = sample.frames.size() / pitch;
            QVERIFY2(
                std::abs(frames - expected) <= 2.0, qPrintable(QStringLiteral("pitch %1: %2 vs %3").arg(pitch).arg(frames).arg(expected)));
        }
    }

    void variationBounds()
    {
        QRandomGenerator generator(1234);
        float minPitch = 2.0f;
        float maxPitch = 0.0f;
        float minGain = 2.0f;
        float maxGain = 0.0f;
        for (int i = 0; i < 20000; ++i) {
            const auto v = KeySound::variation(true, generator);
            QVERIFY(v.pitch >= 1.0f - KeySound::PitchRange && v.pitch <= 1.0f + KeySound::PitchRange);
            QVERIFY(v.gain >= 1.0f - KeySound::GainRange && v.gain <= 1.0f + KeySound::GainRange);
            minPitch = std::min(minPitch, v.pitch);
            maxPitch = std::max(maxPitch, v.pitch);
            minGain = std::min(minGain, v.gain);
            maxGain = std::max(maxGain, v.gain);
        }
        QVERIFY(minPitch < 0.975f && maxPitch > 1.025f);
        QVERIFY(minGain < 0.86f && maxGain > 1.14f);
        QCOMPARE(KeySound::PitchRange, 0.03f);
        QCOMPARE(KeySound::GainRange, 0.15f);

        const auto fixed = KeySound::variation(false, generator);
        QCOMPARE(fixed.pitch, 1.0f);
        QCOMPARE(fixed.gain, 1.0f);
    }

    void qmlSingleton()
    {
        qputenv("KBOARD_USE_BUILD_TREE", "1");
        QQmlEngine engine;
        engine.addImportPath(QStringLiteral(KBOARD_QML_BUILD_DIR));
        QQmlComponent component(&engine);
        component.setData(QByteArrayLiteral("import QtQml\nimport org.devl0rd.kboard.sound\nQtObject {\n"
                                            "    property int packCount: KeySound.packs.length\n"
                                            "    property string firstPack: KeySound.packs[0].id\n"
                                            "    property string current: KeySound.currentPack\n"
                                            "    property bool open: KeySound.deviceOpen\n"
                                            "}\n"),
            QUrl());
        std::unique_ptr<QObject> object(component.create());
        QVERIFY2(object, qPrintable(component.errorString()));
        QVERIFY(object->property("packCount").toInt() >= 5);
        QCOMPARE(object->property("firstPack").toString(), QStringLiteral("soft"));
        QCOMPARE(object->property("current").toString(), QStringLiteral("soft"));
        QCOMPARE(object->property("open").toBool(), false);
    }

    void unknownKindIsIgnored()
    {
        KeySound sound(KeySound::Output::Offline, soundsDir());
        QTest::ignoreMessage(QtWarningMsg, QRegularExpression(QStringLiteral("Unknown sound kind")));
        sound.play(QStringLiteral("bogus"));
        QVERIFY(peak(sound.renderOffline(4800)) == 0.0f);
    }
};

QTEST_GUILESS_MAIN(SoundTest)

#include "soundtest.moc"
