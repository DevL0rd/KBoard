#pragma once

#include <QByteArray>
#include <QDateTime>
#include <QJsonObject>
#include <QSize>
#include <QString>

#include <optional>

struct ClipboardEntry
{
    QString id;
    QString text;
    QString mimeType;
    bool image = false;
    QByteArray imageHash;
    QSize imageSize;
    bool pinned = false;
    QDateTime timestamp;
    bool expiring = false;
    QString source;
    bool isUrl = false;
    QString domain;
    QString otpCode;

    static ClipboardEntry forText(const QString &text, const QDateTime &timestamp, const QString &source);
    static ClipboardEntry forImage(
        const QByteArray &hash, QSize size, const QString &mimeType, const QDateTime &timestamp, const QString &source);
    static std::optional<ClipboardEntry> fromJson(const QJsonObject &object);

    QString kind() const;
    QJsonObject toJson() const;
    void classify();
};
