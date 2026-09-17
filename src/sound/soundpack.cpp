#include "soundpack.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtEndian>

#include <algorithm>

namespace
{
constexpr std::array<const char *, static_cast<size_t>(SoundKind::Count)> kindNames = {
    "key", "space", "backspace", "enter", "modifier", "popup", "open", "close",
};

quint32 readU32(const char *data)
{
    return qFromLittleEndian<quint32>(data);
}

quint16 readU16(const char *data)
{
    return qFromLittleEndian<quint16>(data);
}
}

bool SoundPackLoader::kindFromString(const QString &name, SoundKind *kind)
{
    for (size_t i = 0; i < kindNames.size(); ++i) {
        if (name == QLatin1StringView(kindNames[i])) {
            *kind = static_cast<SoundKind>(i);
            return true;
        }
    }
    return false;
}

QString SoundPackLoader::kindName(SoundKind kind)
{
    return QString::fromLatin1(kindNames[static_cast<size_t>(kind)]);
}

QStringList SoundPackLoader::requiredFiles()
{
    QStringList files{QStringLiteral("pack.json"), QStringLiteral("key1.wav")};
    for (size_t i = 1; i < kindNames.size(); ++i) {
        files << QString::fromLatin1(kindNames[i]) + QStringLiteral(".wav");
    }
    return files;
}

bool SoundPackLoader::readWav(const QString &path, SoundSample *sample, QString *error)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        *error = QStringLiteral("Cannot open %1: %2").arg(path, file.errorString());
        return false;
    }
    const QByteArray bytes = file.readAll();
    if (bytes.size() < 12 || !bytes.startsWith("RIFF") || bytes.mid(8, 4) != "WAVE") {
        *error = QStringLiteral("%1 is not a RIFF WAVE file").arg(path);
        return false;
    }

    int channels = 0;
    int rate = 0;
    int bits = 0;
    int format = 0;
    QByteArrayView pcm;
    qsizetype offset = 12;
    while (offset + 8 <= bytes.size()) {
        const QByteArrayView id(bytes.constData() + offset, 4);
        const qsizetype size = readU32(bytes.constData() + offset + 4);
        const qsizetype body = offset + 8;
        if (body + size > bytes.size()) {
            *error = QStringLiteral("%1 has a truncated chunk").arg(path);
            return false;
        }
        if (id == "fmt " && size >= 16) {
            format = readU16(bytes.constData() + body);
            channels = readU16(bytes.constData() + body + 2);
            rate = static_cast<int>(readU32(bytes.constData() + body + 4));
            bits = readU16(bytes.constData() + body + 14);
        } else if (id == "data") {
            pcm = QByteArrayView(bytes.constData() + body, size);
        }
        offset = body + size + (size & 1);
    }

    if (format != 1 || channels != 1 || bits != 16 || rate != RequiredSampleRate) {
        *error = QStringLiteral("%1 must be 48 kHz mono 16-bit PCM (got format %2, %3 ch, %4 Hz, %5 bit)")
                     .arg(path)
                     .arg(format)
                     .arg(channels)
                     .arg(rate)
                     .arg(bits);
        return false;
    }
    if (pcm.size() < 2) {
        *error = QStringLiteral("%1 has no audio data").arg(path);
        return false;
    }

    const qsizetype count = pcm.size() / 2;
    sample->frames.resize(count);
    for (qsizetype i = 0; i < count; ++i) {
        sample->frames[i] = qFromLittleEndian<qint16>(pcm.data() + i * 2) / 32768.0f;
    }
    sample->sampleRate = rate;
    sample->fileName = QFileInfo(path).fileName();
    return true;
}

std::shared_ptr<const SoundPack> SoundPackLoader::loadPack(const QString &directory, QString *error)
{
    const QDir dir(directory);
    QFile meta(dir.filePath(QStringLiteral("pack.json")));
    if (!meta.open(QIODevice::ReadOnly)) {
        *error = QStringLiteral("Sound pack %1 has no pack.json").arg(directory);
        return {};
    }
    QJsonParseError parseError;
    const QJsonObject object = QJsonDocument::fromJson(meta.readAll(), &parseError).object();
    if (parseError.error != QJsonParseError::NoError) {
        *error = QStringLiteral("%1: %2").arg(meta.fileName(), parseError.errorString());
        return {};
    }

    auto pack = std::make_shared<SoundPack>();
    pack->id = object.value(QLatin1StringView("id")).toString();
    pack->name = object.value(QLatin1StringView("name")).toString();
    pack->description = object.value(QLatin1StringView("description")).toString();
    pack->order = object.value(QLatin1StringView("order")).toInt();
    if (pack->id.isEmpty() || pack->name.isEmpty()) {
        *error = QStringLiteral("%1 needs an id and a name").arg(meta.fileName());
        return {};
    }
    if (pack->id != dir.dirName()) {
        *error = QStringLiteral("%1 id \"%2\" does not match its directory").arg(meta.fileName(), pack->id);
        return {};
    }

    for (int i = 1; i <= MaxKeyVariants; ++i) {
        const QString path = dir.filePath(QStringLiteral("key%1.wav").arg(i));
        if (!QFile::exists(path)) {
            break;
        }
        SoundSample sample;
        if (!readWav(path, &sample, error)) {
            return {};
        }
        pack->samples[static_cast<size_t>(SoundKind::Key)].push_back(std::move(sample));
    }
    if (pack->variants(SoundKind::Key).empty()) {
        *error = QStringLiteral("Sound pack %1 has no key1.wav").arg(pack->id);
        return {};
    }

    for (size_t i = 1; i < kindNames.size(); ++i) {
        SoundSample sample;
        if (!readWav(dir.filePath(QString::fromLatin1(kindNames[i]) + QStringLiteral(".wav")), &sample, error)) {
            return {};
        }
        pack->samples[i].push_back(std::move(sample));
    }
    return pack;
}

QList<std::shared_ptr<const SoundPack>> SoundPackLoader::loadAll(const QString &root, QStringList *errors)
{
    QList<std::shared_ptr<const SoundPack>> packs;
    const QDir dir(root);
    if (!dir.exists()) {
        errors->append(QStringLiteral("Sound pack directory %1 does not exist").arg(root));
        return packs;
    }
    const QStringList entries = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    for (const QString &entry : entries) {
        if (!QFile::exists(dir.filePath(entry + QStringLiteral("/pack.json")))) {
            continue;
        }
        QString error;
        if (auto pack = loadPack(dir.filePath(entry), &error)) {
            packs.append(std::move(pack));
        } else {
            errors->append(error);
        }
    }
    std::sort(packs.begin(), packs.end(), [](const auto &a, const auto &b) {
        return a->order != b->order ? a->order < b->order : a->id < b->id;
    });
    return packs;
}
