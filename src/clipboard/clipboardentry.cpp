#include "clipboardentry.h"
#include "clipboarddetect.h"

#include <QTimeZone>
#include <QUuid>

namespace
{
ClipboardEntry baseEntry(const QString &mimeType, const QDateTime &timestamp, const QString &source)
{
    ClipboardEntry entry;
    entry.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    entry.mimeType = mimeType;
    entry.timestamp = timestamp;
    entry.source = source;
    return entry;
}
}

ClipboardEntry ClipboardEntry::forText(const QString &text, const QDateTime &timestamp, const QString &source)
{
    ClipboardEntry entry = baseEntry(QStringLiteral("text/plain"), timestamp, source);
    entry.text = text;
    entry.classify();
    return entry;
}

ClipboardEntry ClipboardEntry::forImage(
    const QByteArray &hash, QSize size, const QString &mimeType, const QDateTime &timestamp, const QString &source)
{
    ClipboardEntry entry = baseEntry(mimeType, timestamp, source);
    entry.image = true;
    entry.imageHash = hash;
    entry.imageSize = size;
    return entry;
}

std::optional<ClipboardEntry> ClipboardEntry::fromJson(const QJsonObject &object)
{
    ClipboardEntry entry;
    entry.id = object.value(QLatin1String("id")).toString();
    entry.mimeType = object.value(QLatin1String("mimeType")).toString();
    entry.pinned = object.value(QLatin1String("pinned")).toBool();
    entry.timestamp = QDateTime::fromMSecsSinceEpoch(qint64(object.value(QLatin1String("timestamp")).toDouble()), QTimeZone::UTC);
    entry.source = object.value(QLatin1String("source")).toString();
    entry.image = object.value(QLatin1String("kind")).toString() == QLatin1String("image");
    if (entry.image) {
        entry.imageHash = object.value(QLatin1String("imageHash")).toString().toLatin1();
        entry.imageSize = QSize(object.value(QLatin1String("imageWidth")).toInt(), object.value(QLatin1String("imageHeight")).toInt());
    } else {
        entry.text = object.value(QLatin1String("text")).toString();
    }
    if (entry.id.isEmpty() || (!entry.image && entry.text.isEmpty())) {
        return std::nullopt;
    }
    entry.classify();
    return entry;
}

QString ClipboardEntry::kind() const
{
    if (image) {
        return QStringLiteral("image");
    }
    if (isUrl) {
        return QStringLiteral("url");
    }
    return otpCode.isEmpty() ? QStringLiteral("text") : QStringLiteral("otp");
}

QJsonObject ClipboardEntry::toJson() const
{
    QJsonObject object {
        {QStringLiteral("id"), id},
        {QStringLiteral("mimeType"), mimeType},
        {QStringLiteral("pinned"), pinned},
        {QStringLiteral("timestamp"), timestamp.toMSecsSinceEpoch()},
        {QStringLiteral("source"), source},
        {QStringLiteral("kind"), image ? QStringLiteral("image") : QStringLiteral("text")},
    };
    if (image) {
        object.insert(QStringLiteral("imageHash"), QString::fromLatin1(imageHash));
        object.insert(QStringLiteral("imageWidth"), imageSize.width());
        object.insert(QStringLiteral("imageHeight"), imageSize.height());
    } else {
        object.insert(QStringLiteral("text"), text);
    }
    return object;
}

void ClipboardEntry::classify()
{
    isUrl = !image && ClipboardDetect::isWebUrl(text);
    domain = isUrl ? ClipboardDetect::domainOf(text) : QString();
    otpCode = (image || isUrl) ? QString() : ClipboardDetect::otpCode(text);
}
