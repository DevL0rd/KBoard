#pragma once

#include <QImage>
#include <QString>

class QMimeData;

namespace ClipboardDetect
{
struct CopiedContent
{
    bool secret = false;
    QImage image;
    QString imageMimeType;
    QString text;
};

CopiedContent read(const QMimeData *mime);
bool isSecret(const QMimeData *mime);
QString otpCode(const QString &text);
bool isWebUrl(const QString &text);
QString domainOf(const QString &text);
bool isKlipperImagePlaceholder(const QString &text);
}
