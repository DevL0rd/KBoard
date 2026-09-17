#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QString>
#include <QStringList>
#include <QVariantMap>

struct ModelEntry
{
    QString id;
    QString name;
    QString variant;
    QString engine;
    QString file;
    QString url;
    QString sha256;
    QString tier;
    QString description;
    QString license;
    QString languageSet;
    QStringList languages;
    qint64 sizeBytes = 0;
    double speed = 0.0;
    double realTimeFactorCpu = 0.0;
    double realTimeFactorGpu = 0.0;
    bool recommended = false;

    bool isValid() const;
    QVariantMap toVariantMap() const;
};

class ModelCatalog
{
public:
    static ModelCatalog fromJson(const QByteArray &json, QString *error);
    static ModelCatalog fromFile(const QString &path, QString *error);
    static QString defaultCatalogPath();
    static QString defaultModelsDirectory();

    bool isValid() const;
    const QList<ModelEntry> &models() const;
    const ModelEntry &vad() const;
    QString defaultModelId() const;
    QString speedReference() const;
    const ModelEntry *find(const QString &id) const;

    static QString pathFor(const ModelEntry &entry, const QString &directory);
    static bool isDownloaded(const ModelEntry &entry, const QString &directory);

private:
    QString parseModels(const QJsonArray &models, const QJsonObject &languageSets);
    QString parseVad(const QJsonObject &vad);
    QString parseDefault(const QString &id);
    void setSpeedReference(const QString &reference);

    QList<ModelEntry> m_models;
    ModelEntry m_vad;
    QString m_defaultModelId;
    QString m_speedReference;
};
