#pragma once

#include "clipboardentry.h"

#include <QHash>
#include <QVariant>

namespace ClipboardRoles
{
enum Role
{
    IdRole = Qt::UserRole + 1,
    TextRole,
    FullTextRole,
    MimeTypeRole,
    KindRole,
    ImageSourceRole,
    ThumbnailSourceRole,
    ImageWidthRole,
    ImageHeightRole,
    PinnedRole,
    TimestampRole,
    IsUrlRole,
    DomainRole,
    IsOtpRole,
    OtpCodeRole,
    ExpiringRole,
    SourceRole,
};

const QString &imageProviderId();
QHash<int, QByteArray> names();
QVariant value(const ClipboardEntry &entry, int role);
QVariantMap toMap(const ClipboardEntry &entry);
}
