#include "voicebackends.h"

#include <ggml-backend.h>

#include <QRegularExpression>

#include <algorithm>
#include <mutex>

bool VoiceDevice::isGpu() const
{
    return type == u"gpu" || type == u"igpu";
}

QString VoiceDevice::label() const
{
    if (!isGpu()) {
        return QStringLiteral("CPU");
    }
    return QStringLiteral("%1 · %2").arg(backend, VoiceBackends::shortDescription(description));
}

QVariantMap VoiceDevice::toVariantMap() const
{
    return {
        {QStringLiteral("name"), name},
        {QStringLiteral("description"), description},
        {QStringLiteral("type"), type},
        {QStringLiteral("backend"), backend},
        {QStringLiteral("label"), label()},
        {QStringLiteral("memoryTotal"), memoryTotal},
    };
}

namespace VoiceBackends
{
void ensureLoaded()
{
#ifdef KBOARD_LOAD_GGML_BACKENDS
    static std::once_flag once;
    std::call_once(once, [] { ggml_backend_load_all(); });
#endif
}

QList<VoiceDevice> devices()
{
    ensureLoaded();
    QList<VoiceDevice> result;
    int gpuCounter = 0;
    for (size_t i = 0; i < ggml_backend_dev_count(); ++i) {
        ggml_backend_dev_t dev = ggml_backend_dev_get(i);
        VoiceDevice device;
        device.name = QString::fromUtf8(ggml_backend_dev_name(dev));
        device.description = QString::fromUtf8(ggml_backend_dev_description(dev));
        device.backend = QString::fromUtf8(ggml_backend_reg_name(ggml_backend_dev_backend_reg(dev)));
        switch (ggml_backend_dev_type(dev)) {
        case GGML_BACKEND_DEVICE_TYPE_CPU:
            device.type = QStringLiteral("cpu");
            break;
        case GGML_BACKEND_DEVICE_TYPE_GPU:
            device.type = QStringLiteral("gpu");
            break;
        case GGML_BACKEND_DEVICE_TYPE_IGPU:
            device.type = QStringLiteral("igpu");
            break;
        case GGML_BACKEND_DEVICE_TYPE_ACCEL:
            device.type = QStringLiteral("accel");
            break;
        case GGML_BACKEND_DEVICE_TYPE_META:
            device.type = QStringLiteral("meta");
            break;
        }
        if (device.isGpu()) {
            device.gpuIndex = gpuCounter++;
            size_t freeMemory = 0;
            size_t totalMemory = 0;
            ggml_backend_dev_memory(dev, &freeMemory, &totalMemory);
            device.memoryTotal = qint64(totalMemory);
        }
        if (device.type == u"accel" || device.type == u"meta") {
            continue;
        }
        result.append(device);
    }
    return result;
}

int backendPriority(const QString &backend)
{
    const QString lower = backend.toLower();
    if (lower == u"cuda") {
        return 0;
    }
    if (lower == u"rocm" || lower == u"hip" || lower == u"musa") {
        return 1;
    }
    if (lower == u"sycl" || lower == u"openvino") {
        return 2;
    }
    if (lower == u"vulkan") {
        return 3;
    }
    if (lower == u"cpu") {
        return 100;
    }
    return 4;
}

QList<VoiceDevice> ranked(const QList<VoiceDevice> &devices)
{
    QList<VoiceDevice> result = devices;
    const auto typeRank = [](const VoiceDevice &device) {
        if (device.type == u"gpu") {
            return 0;
        }
        if (device.type == u"igpu") {
            return 1;
        }
        return 2;
    };
    std::stable_sort(result.begin(), result.end(), [&](const VoiceDevice &a, const VoiceDevice &b) {
        if (typeRank(a) != typeRank(b)) {
            return typeRank(a) < typeRank(b);
        }
        if (backendPriority(a.backend) != backendPriority(b.backend)) {
            return backendPriority(a.backend) < backendPriority(b.backend);
        }
        return a.memoryTotal > b.memoryTotal;
    });
    return result;
}

VoiceDevice select(const QList<VoiceDevice> &devices, const QString &setting, QString *error)
{
    const QString wanted = setting.trimmed();
    if (wanted.isEmpty() || wanted.compare(u"auto", Qt::CaseInsensitive) == 0) {
        const QList<VoiceDevice> order = ranked(devices);
        if (order.isEmpty()) {
            if (error) {
                *error = QStringLiteral("No compute device is available for voice typing");
            }
            return {};
        }
        return order.first();
    }
    for (const VoiceDevice &device : devices) {
        if (device.name.compare(wanted, Qt::CaseInsensitive) == 0) {
            return device;
        }
    }
    if (error) {
        QStringList names;
        for (const VoiceDevice &device : devices) {
            names.append(device.name);
        }
        *error = QStringLiteral("Voice device \"%1\" was not found (available: %2)").arg(wanted, names.join(QStringLiteral(", ")));
    }
    return {};
}

QString shortDescription(const QString &description)
{
    QString result = description;
    static const QRegularExpression vendor(QStringLiteral("^(NVIDIA\\s+GeForce\\s+|NVIDIA\\s+|AMD\\s+|Intel\\(R\\)\\s+|Intel\\s+)"));
    result.remove(vendor);
    result.remove(QStringLiteral("(TM)"));
    result.remove(QStringLiteral("(R)"));
    return result.simplified();
}

bool probe(const VoiceDevice &device, QString *error)
{
    ensureLoaded();
    ggml_backend_dev_t dev = ggml_backend_dev_by_name(device.name.toUtf8().constData());
    if (!dev) {
        if (error) {
            *error = QStringLiteral("Voice device %1 disappeared").arg(device.name);
        }
        return false;
    }
    ggml_backend_t backend = ggml_backend_dev_init(dev, nullptr);
    if (!backend) {
        if (error) {
            *error = QStringLiteral("Could not start %1 on %2 (%3)").arg(device.backend, device.description, device.name);
        }
        return false;
    }
    ggml_backend_free(backend);
    return true;
}
}
