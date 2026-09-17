#include "clipboardroles.h"

#include <functional>

namespace
{
constexpr int s_displayLimit = 1000;

QString imageUrl(const ClipboardEntry &entry, const QString &variant)
{
    return entry.image ? QStringLiteral("image://%1/%2/%3").arg(ClipboardRoles::imageProviderId(), entry.id, variant) : QString();
}

QString displayText(const ClipboardEntry &entry)
{
    return entry.text.size() > s_displayLimit ? entry.text.left(s_displayLimit) + QStringLiteral("…") : entry.text;
}

struct RoleSpec
{
    QByteArray name;
    std::function<QVariant(const ClipboardEntry &)> value;
};

const QHash<int, RoleSpec> &specs()
{
    using namespace ClipboardRoles;
    static const QHash<int, RoleSpec> table {
        {IdRole, {"itemId", [](const ClipboardEntry &e) { return QVariant(e.id); }}},
        {TextRole, {"text", [](const ClipboardEntry &e) { return QVariant(displayText(e)); }}},
        {FullTextRole, {"fullText", [](const ClipboardEntry &e) { return QVariant(e.text); }}},
        {MimeTypeRole, {"mimeType", [](const ClipboardEntry &e) { return QVariant(e.mimeType); }}},
        {KindRole, {"kind", [](const ClipboardEntry &e) { return QVariant(e.kind()); }}},
        {ImageSourceRole, {"imageSource", [](const ClipboardEntry &e) { return QVariant(imageUrl(e, QStringLiteral("full"))); }}},
        {ThumbnailSourceRole, {"thumbnailSource", [](const ClipboardEntry &e) { return QVariant(imageUrl(e, QStringLiteral("thumb"))); }}},
        {ImageWidthRole, {"imageWidth", [](const ClipboardEntry &e) { return QVariant(e.imageSize.width()); }}},
        {ImageHeightRole, {"imageHeight", [](const ClipboardEntry &e) { return QVariant(e.imageSize.height()); }}},
        {PinnedRole, {"pinned", [](const ClipboardEntry &e) { return QVariant(e.pinned); }}},
        {TimestampRole, {"timestamp", [](const ClipboardEntry &e) { return QVariant(e.timestamp); }}},
        {IsUrlRole, {"isUrl", [](const ClipboardEntry &e) { return QVariant(e.isUrl); }}},
        {DomainRole, {"domain", [](const ClipboardEntry &e) { return QVariant(e.domain); }}},
        {IsOtpRole, {"isOtp", [](const ClipboardEntry &e) { return QVariant(!e.otpCode.isEmpty()); }}},
        {OtpCodeRole, {"otpCode", [](const ClipboardEntry &e) { return QVariant(e.otpCode); }}},
        {ExpiringRole, {"expiring", [](const ClipboardEntry &e) { return QVariant(e.expiring); }}},
        {SourceRole, {"source", [](const ClipboardEntry &e) { return QVariant(e.source); }}},
    };
    return table;
}
}

namespace ClipboardRoles
{
const QString &imageProviderId()
{
    static const QString id = QStringLiteral("kboardclipboard");
    return id;
}

QHash<int, QByteArray> names()
{
    QHash<int, QByteArray> result;
    for (auto it = specs().cbegin(); it != specs().cend(); ++it) {
        result.insert(it.key(), it->name);
    }
    return result;
}

QVariant value(const ClipboardEntry &entry, int role)
{
    const int resolved = role == Qt::DisplayRole ? int(TextRole) : role;
    const auto it = specs().constFind(resolved);
    return it == specs().cend() ? QVariant() : it->value(entry);
}

QVariantMap toMap(const ClipboardEntry &entry)
{
    QVariantMap map;
    for (auto it = specs().cbegin(); it != specs().cend(); ++it) {
        map.insert(QString::fromLatin1(it->name), it->value(entry));
    }
    return map;
}
}
