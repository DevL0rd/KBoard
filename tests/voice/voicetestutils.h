#pragma once

#include "modelcatalog.h"

#include <QFile>
#include <QList>
#include <QString>
#include <QtEndian>

#include <cmath>
#include <random>

namespace VoiceTest
{
inline QString dataPath(const QString &name)
{
    return QStringLiteral(KBOARD_VOICE_TEST_DATA "/") + name;
}

inline QList<float> readWav16kMono(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }
    const QByteArray bytes = file.readAll();
    qsizetype offset = 12;
    while (offset + 8 <= bytes.size()) {
        const QByteArray id = bytes.mid(offset, 4);
        const quint32 size = qFromLittleEndian<quint32>(bytes.constData() + offset + 4);
        if (id == "data") {
            QList<float> samples;
            const qsizetype count = qsizetype(size) / 2;
            samples.reserve(count);
            for (qsizetype i = 0; i < count; ++i) {
                samples.append(float(qFromLittleEndian<qint16>(bytes.constData() + offset + 8 + 2 * i)) / 32768.0f);
            }
            return samples;
        }
        offset += 8 + qsizetype(size);
    }
    return {};
}

inline QList<float> silence(double seconds)
{
    return QList<float>(qsizetype(seconds * 16000.0), 0.0f);
}

inline QList<float> noise(double seconds, float amplitude)
{
    std::mt19937 generator(7);
    std::uniform_real_distribution<float> distribution(-amplitude, amplitude);
    QList<float> samples(qsizetype(seconds * 16000.0));
    for (float &sample : samples) {
        sample = distribution(generator);
    }
    return samples;
}

inline QList<float> tone(double seconds, double frequency, float amplitude)
{
    QList<float> samples(qsizetype(seconds * 16000.0));
    for (qsizetype i = 0; i < samples.size(); ++i) {
        samples[i] = amplitude * float(std::sin(2.0 * M_PI * frequency * double(i) / 16000.0));
    }
    return samples;
}

inline QString installedModelPath(const QString &id)
{
    QString error;
    const ModelCatalog catalog = ModelCatalog::fromFile(QStringLiteral(KBOARD_DATA_BUILD_DIR "/voice/models.json"), &error);
    const ModelEntry *entry = catalog.find(id);
    if (!entry || !ModelCatalog::isDownloaded(*entry, ModelCatalog::defaultModelsDirectory())) {
        return {};
    }
    return ModelCatalog::pathFor(*entry, ModelCatalog::defaultModelsDirectory());
}
}
