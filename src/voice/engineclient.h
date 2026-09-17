#pragma once

#include "speechengine.h"

#include <QList>
#include <QObject>
#include <QVariantList>

#include <atomic>
#include <functional>
#include <memory>

class QThread;

struct DecodeRequest
{
    quint64 session = 0;
    quint64 utterance = 0;
    bool final = false;
    QList<float> samples;
    QString language;
};

class EngineClient : public QObject
{
    Q_OBJECT

public:
    explicit EngineClient(QObject *parent = nullptr);
    ~EngineClient() override;

    void load(const QString &engine, const QString &modelPath, const QString &deviceSetting);
    void unload();
    bool isReady() const;
    bool isBusy() const;
    bool matches(const QString &modelPath, const QString &deviceSetting) const;
    QString loadedPath() const;
    VoiceDevice device() const;

    void transcribe(const DecodeRequest &request);
    void dropPendingPartials();

    void refreshDevices();
    bool hasDevices() const;
    QVariantList devices() const;

Q_SIGNALS:
    void readyChanged();
    void loadFailed(const QString &error);
    void decoded(const DecodeRequest &request, const SpeechEngine::Result &result);
    void devicesChanged();

private:
    struct Host;

    void post(std::function<void(SpeechEngine &)> job);

    QThread *m_thread;
    Host *m_host;
    std::shared_ptr<std::atomic<quint64>> m_latestSerial;
    quint64 m_serial = 0;
    quint64 m_loadToken = 0;
    bool m_ready = false;
    bool m_loading = false;
    QString m_path;
    QString m_deviceSetting;
    VoiceDevice m_device;
    bool m_devicesRequested = false;
    QVariantList m_devices;
};
