#pragma once

#include <QList>
#include <QString>
#include <QVariantMap>

struct VoiceDevice
{
    QString name;
    QString description;
    QString type;
    QString backend;
    qint64 memoryTotal = 0;
    int gpuIndex = -1;

    bool isGpu() const;
    QString label() const;
    QVariantMap toVariantMap() const;
};

namespace VoiceBackends
{
void ensureLoaded();
QList<VoiceDevice> devices();
int backendPriority(const QString &backend);
QList<VoiceDevice> ranked(const QList<VoiceDevice> &devices);
VoiceDevice select(const QList<VoiceDevice> &devices, const QString &setting, QString *error);
QString shortDescription(const QString &description);
bool probe(const VoiceDevice &device, QString *error);
}
