#include "engineclient.h"

#include <QThread>

struct EngineClient::Host : public QObject
{
    SpeechEngine engine;
};

EngineClient::EngineClient(QObject *parent)
    : QObject(parent)
    , m_thread(new QThread(this))
    , m_host(new Host)
    , m_latestSerial(std::make_shared<std::atomic<quint64>>(0))
{
    m_thread->setObjectName(QStringLiteral("KBoard voice engine"));
    m_host->moveToThread(m_thread);
    m_thread->start();
}

EngineClient::~EngineClient()
{
    QMetaObject::invokeMethod(m_host, [host = m_host]() { delete host; }, Qt::BlockingQueuedConnection);
    m_thread->quit();
    m_thread->wait();
}

void EngineClient::post(std::function<void(SpeechEngine &)> job)
{
    QMetaObject::invokeMethod(m_host, [host = m_host, job = std::move(job)]() { job(host->engine); }, Qt::QueuedConnection);
}

void EngineClient::load(const QString &engine, const QString &modelPath, const QString &deviceSetting)
{
    if ((m_ready || m_loading) && matches(modelPath, deviceSetting)) {
        return;
    }
    const bool wasReady = m_ready;
    m_ready = false;
    m_loading = true;
    m_path = modelPath;
    m_deviceSetting = deviceSetting;
    const quint64 token = ++m_loadToken;
    if (wasReady) {
        Q_EMIT readyChanged();
    }
    post([this, token, engine, modelPath, deviceSetting](SpeechEngine &speech) {
        QString error;
        const bool ok = speech.load(engine, modelPath, deviceSetting, &error);
        const VoiceDevice used = speech.device();
        QMetaObject::invokeMethod(
            this,
            [this, token, ok, error, used]() {
                if (token != m_loadToken) {
                    return;
                }
                m_loading = false;
                if (!ok) {
                    m_path.clear();
                    m_deviceSetting.clear();
                    Q_EMIT loadFailed(error);
                    return;
                }
                m_ready = true;
                m_device = used;
                Q_EMIT readyChanged();
            },
            Qt::QueuedConnection);
    });
}

void EngineClient::unload()
{
    ++m_loadToken;
    const bool wasReady = m_ready;
    m_ready = false;
    m_loading = false;
    m_path.clear();
    m_deviceSetting.clear();
    m_device = {};
    post([](SpeechEngine &speech) { speech.unload(); });
    if (wasReady) {
        Q_EMIT readyChanged();
    }
}

bool EngineClient::isReady() const
{
    return m_ready;
}

bool EngineClient::isBusy() const
{
    return m_ready || m_loading;
}

bool EngineClient::matches(const QString &modelPath, const QString &deviceSetting) const
{
    return m_path == modelPath && m_deviceSetting == deviceSetting;
}

QString EngineClient::loadedPath() const
{
    return m_ready ? m_path : QString();
}

VoiceDevice EngineClient::device() const
{
    return m_device;
}

void EngineClient::transcribe(const DecodeRequest &request)
{
    const quint64 serial = ++m_serial;
    m_latestSerial->store(serial);
    post([this, latest = m_latestSerial, serial, request](SpeechEngine &speech) {
        if (!request.final && latest->load() != serial) {
            return;
        }
        SpeechEngine::Result result;
        if (speech.isLoaded()) {
            result = speech.transcribe(request.samples, request.language);
        } else {
            result.error = QStringLiteral("The voice model is not loaded");
        }
        QMetaObject::invokeMethod(this, [this, request, result]() { Q_EMIT decoded(request, result); }, Qt::QueuedConnection);
    });
}

void EngineClient::dropPendingPartials()
{
    m_latestSerial->store(++m_serial);
}

void EngineClient::refreshDevices()
{
    m_devicesRequested = true;
    post([this](SpeechEngine &) {
        SpeechEngine::installLogHandler();
        QVariantList list;
        const QList<VoiceDevice> devices = VoiceBackends::ranked(VoiceBackends::devices());
        for (const VoiceDevice &device : devices) {
            list.append(device.toVariantMap());
        }
        QMetaObject::invokeMethod(
            this,
            [this, list]() {
                m_devices = list;
                Q_EMIT devicesChanged();
            },
            Qt::QueuedConnection);
    });
}

bool EngineClient::hasDevices() const
{
    return m_devicesRequested;
}

QVariantList EngineClient::devices() const
{
    return m_devices;
}
