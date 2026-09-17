#include "modelcatalog.h"

#include "kboardpaths.h"

#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>

namespace
{
ModelEntry entryFromJson(const QJsonObject &object)
{
    ModelEntry entry;
    entry.id = object.value(u"id").toString();
    entry.name = object.value(u"name").toString();
    entry.variant = object.value(u"variant").toString();
    entry.engine = object.value(u"engine").toString();
    entry.file = object.value(u"file").toString();
    entry.url = object.value(u"url").toString();
    entry.sha256 = object.value(u"sha256").toString().toLower();
    entry.tier = object.value(u"tier").toString();
    entry.description = object.value(u"description").toString();
    entry.license = object.value(u"license").toString();
    entry.sizeBytes = object.value(u"sizeBytes").toInteger();
    entry.recommended = object.value(u"recommended").toBool();
    entry.languageSet = object.value(u"languageSet").toString();
    return entry;
}

QString validationError(const ModelEntry &entry)
{
    static const QStringList engines {QStringLiteral("parakeet"), QStringLiteral("whisper"), QStringLiteral("silero-vad")};
    static const QRegularExpression sha(QStringLiteral("^[0-9a-f]{64}$"));
    if (entry.id.isEmpty()) {
        return QStringLiteral("a model has no id");
    }
    if (!engines.contains(entry.engine)) {
        return QStringLiteral("model %1 has unknown engine \"%2\"").arg(entry.id, entry.engine);
    }
    if (entry.file.isEmpty() || entry.file.contains(u'/')) {
        return QStringLiteral("model %1 has an invalid file name").arg(entry.id);
    }
    if (!entry.url.startsWith(u"https://")) {
        return QStringLiteral("model %1 needs an https download URL").arg(entry.id);
    }
    if (entry.sizeBytes <= 0) {
        return QStringLiteral("model %1 has no size").arg(entry.id);
    }
    if (!entry.sha256.isEmpty() && !sha.match(entry.sha256).hasMatch()) {
        return QStringLiteral("model %1 has an invalid sha256").arg(entry.id);
    }
    return {};
}
}

bool ModelEntry::isValid() const
{
    return !id.isEmpty();
}

QVariantMap ModelEntry::toVariantMap() const
{
    return {
        {QStringLiteral("id"), id},
        {QStringLiteral("name"), name},
        {QStringLiteral("variant"), variant},
        {QStringLiteral("engine"), engine},
        {QStringLiteral("file"), file},
        {QStringLiteral("url"), url},
        {QStringLiteral("sha256"), sha256},
        {QStringLiteral("tier"), tier},
        {QStringLiteral("description"), description},
        {QStringLiteral("license"), license},
        {QStringLiteral("languages"), languages},
        {QStringLiteral("sizeBytes"), sizeBytes},
        {QStringLiteral("sizeMb"), qRound64(double(sizeBytes) / (1024.0 * 1024.0))},
        {QStringLiteral("recommended"), recommended},
    };
}

QString ModelCatalog::parseModels(const QJsonArray &models, const QJsonObject &languageSets)
{
    for (const auto &value : models) {
        ModelEntry entry = entryFromJson(value.toObject());
        const QJsonValue languages = languageSets.value(entry.languageSet);
        entry.languages = languages.toVariant().toStringList();
        QString problem = validationError(entry);
        if (problem.isEmpty() && (!languages.isArray() || entry.languages.isEmpty())) {
            problem = QStringLiteral("model %1 uses unknown language set \"%2\"").arg(entry.id, entry.languageSet);
        }
        if (problem.isEmpty() && entry.engine == u"silero-vad") {
            problem = QStringLiteral("model %1 is a VAD model inside the speech model list").arg(entry.id);
        }
        if (problem.isEmpty() && find(entry.id)) {
            problem = QStringLiteral("model id %1 is listed twice").arg(entry.id);
        }
        if (!problem.isEmpty()) {
            return problem;
        }
        m_models.append(entry);
    }
    return m_models.isEmpty() ? QStringLiteral("no speech models are listed") : QString();
}

QString ModelCatalog::parseVad(const QJsonObject &vad)
{
    m_vad = entryFromJson(vad);
    QString problem = validationError(m_vad);
    if (problem.isEmpty() && m_vad.engine != u"silero-vad") {
        return QStringLiteral("the vad entry must use the silero-vad engine");
    }
    return problem;
}

QString ModelCatalog::parseDefault(const QString &id)
{
    m_defaultModelId = id;
    return find(id) ? QString() : QStringLiteral("default model \"%1\" is not listed").arg(id);
}

ModelCatalog ModelCatalog::fromJson(const QByteArray &json, QString *error)
{
    const auto fail = [error](const QString &message) {
        if (error) {
            *error = message;
        }
        return ModelCatalog();
    };
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(json, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        return fail(QStringLiteral("Voice model catalogue is not valid JSON: %1").arg(parseError.errorString()));
    }
    const QJsonObject root = document.object();
    ModelCatalog catalog;
    QString problem = catalog.parseModels(root.value(u"models").toArray(), root.value(u"languageSets").toObject());
    if (problem.isEmpty()) {
        problem = catalog.parseVad(root.value(u"vad").toObject());
    }
    if (problem.isEmpty()) {
        problem = catalog.parseDefault(root.value(u"defaultModel").toString());
    }
    if (!problem.isEmpty()) {
        return fail(QStringLiteral("Voice model catalogue is invalid: %1").arg(problem));
    }
    return catalog;
}

ModelCatalog ModelCatalog::fromFile(const QString &path, QString *error)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        if (error) {
            *error = QStringLiteral("Voice model catalogue cannot be read: %1 (%2)").arg(path, file.errorString());
        }
        return {};
    }
    return fromJson(file.readAll(), error);
}

QString ModelCatalog::defaultCatalogPath()
{
    return KBoardPaths::dataFile(QStringLiteral("voice/models.json"));
}

QString ModelCatalog::defaultModelsDirectory()
{
    return KBoardPaths::userDataFile(QStringLiteral("models"));
}

bool ModelCatalog::isValid() const
{
    return !m_models.isEmpty() && m_vad.isValid();
}

const QList<ModelEntry> &ModelCatalog::models() const
{
    return m_models;
}

const ModelEntry &ModelCatalog::vad() const
{
    return m_vad;
}

QString ModelCatalog::defaultModelId() const
{
    return m_defaultModelId;
}

const ModelEntry *ModelCatalog::find(const QString &id) const
{
    for (const ModelEntry &entry : m_models) {
        if (entry.id == id) {
            return &entry;
        }
    }
    if (m_vad.isValid() && m_vad.id == id) {
        return &m_vad;
    }
    return nullptr;
}

QString ModelCatalog::pathFor(const ModelEntry &entry, const QString &directory)
{
    return directory + QLatin1Char('/') + entry.file;
}

bool ModelCatalog::isDownloaded(const ModelEntry &entry, const QString &directory)
{
    const QFileInfo info(pathFor(entry, directory));
    return info.isFile() && info.size() == entry.sizeBytes;
}
