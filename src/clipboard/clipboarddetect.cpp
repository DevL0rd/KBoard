#include "clipboarddetect.h"

#include <QMimeData>
#include <QRegularExpression>
#include <QUrl>

#include <algorithm>

namespace
{
const QString s_passwordHint = QStringLiteral("x-kde-passwordManagerHint");

const QRegularExpression &standaloneCode()
{
    static const QRegularExpression re(QStringLiteral("^(\\d{4,8}|\\d{3}[- ]\\d{3}|\\d{4}[- ]\\d{4})$"));
    return re;
}

const QRegularExpression &codeKeyword()
{
    static const QRegularExpression re(
        QStringLiteral("\\b(code|codes|otp|passcode|pin|verification|verify|2fa|mfa|one[- ]time|security|login|log[- ]in|sign[- "
                       "]in|auth(?:entication|enticate|enticator)?|token|confirm\\w*|c[oó]digo|codice|kod|pass)\\b"),
        QRegularExpression::CaseInsensitiveOption | QRegularExpression::UseUnicodePropertiesOption);
    return re;
}

const QRegularExpression &embeddedCode()
{
    static const QRegularExpression re(QStringLiteral("(?<![\\w.,:/$€£¥#+])(?<!\\d-)(\\d{3}[- ]\\d{3}|\\d{4,8})(?![\\w%€$]|[.,:/\\-]\\d)"),
        QRegularExpression::UseUnicodePropertiesOption);
    return re;
}

const QRegularExpression &webUrl()
{
    static const QRegularExpression re(
        QStringLiteral("^((https?|ftp)://[^\\s/$.?#][^\\s]*|www\\.[^\\s/.]+\\.[^\\s]{2,})$"), QRegularExpression::CaseInsensitiveOption);
    return re;
}

QString firstImageFormat(const QMimeData *mime)
{
    const QStringList formats = mime->formats();
    const auto it = std::ranges::find_if(formats, [](const QString &format) { return format.startsWith(QLatin1String("image/")); });
    return it == formats.cend() ? QStringLiteral("image/png") : *it;
}

QString urlsAsText(const QList<QUrl> &urls)
{
    QStringList lines;
    for (const QUrl &url : urls) {
        lines.append(url.isLocalFile() ? url.toLocalFile() : url.toString());
    }
    return lines.join(QLatin1Char('\n'));
}

QString digitsOnly(const QString &code)
{
    QString out;
    for (QChar c : code) {
        if (c.isDigit()) {
            out.append(c);
        }
    }
    return out;
}
}

namespace ClipboardDetect
{
CopiedContent read(const QMimeData *mime)
{
    CopiedContent content;
    if (!mime) {
        return content;
    }
    content.secret = isSecret(mime);
    content.text = mime->hasText() ? mime->text() : (mime->hasUrls() ? urlsAsText(mime->urls()) : QString());
    if (!content.secret && mime->hasImage()) {
        content.image = qvariant_cast<QImage>(mime->imageData());
        content.imageMimeType = firstImageFormat(mime);
    }
    return content;
}

bool isSecret(const QMimeData *mime)
{
    return mime && mime->hasFormat(s_passwordHint) && mime->data(s_passwordHint).trimmed() == QByteArrayLiteral("secret");
}

QString otpCode(const QString &text)
{
    const QString trimmed = text.trimmed();
    if (trimmed.isEmpty() || trimmed.size() > 400) {
        return {};
    }
    if (standaloneCode().match(trimmed).hasMatch()) {
        return digitsOnly(trimmed);
    }
    const auto keyword = codeKeyword().match(trimmed);
    if (!keyword.hasMatch()) {
        return {};
    }
    QString best;
    qsizetype bestDistance = -1;
    auto it = embeddedCode().globalMatch(trimmed);
    while (it.hasNext()) {
        const auto match = it.next();
        const qsizetype distance = qAbs(match.capturedStart(1) - keyword.capturedEnd());
        if (bestDistance < 0 || distance < bestDistance) {
            bestDistance = distance;
            best = digitsOnly(match.captured(1));
        }
    }
    return best;
}

bool isWebUrl(const QString &text)
{
    const QString trimmed = text.trimmed();
    if (trimmed.size() > 4096 || !webUrl().match(trimmed).hasMatch()) {
        return false;
    }
    return !domainOf(trimmed).isEmpty();
}

QString domainOf(const QString &text)
{
    QString trimmed = text.trimmed();
    if (trimmed.startsWith(QLatin1String("www."), Qt::CaseInsensitive)) {
        trimmed.prepend(QLatin1String("https://"));
    }
    QString host = QUrl(trimmed, QUrl::StrictMode).host().toLower();
    if (host.startsWith(QLatin1String("www."))) {
        host.remove(0, 4);
    }
    return host;
}

bool isKlipperImagePlaceholder(const QString &text)
{
    static const QRegularExpression re(QStringLiteral("^▨ \\d+x\\d+"));
    return re.match(text).hasMatch();
}
}
