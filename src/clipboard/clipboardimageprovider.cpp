#include "clipboardimageprovider.h"
#include "clipboardhistory.h"

ClipboardImageProvider::ClipboardImageProvider(ClipboardHistory *history)
    : QQuickImageProvider(QQuickImageProvider::Image)
    , m_history(history)
{ }

QImage ClipboardImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    const qsizetype slash = id.lastIndexOf(QLatin1Char('/'));
    const QString itemId = id.left(slash);
    const bool thumbnail = id.mid(slash + 1) == QLatin1String("thumb");
    QImage image = m_history ? m_history->images().image(itemId, thumbnail) : QImage();
    if (size) {
        *size = image.size();
    }
    if (!image.isNull() && requestedSize.isValid() && (requestedSize.width() < image.width() || requestedSize.height() < image.height())) {
        image = image.scaled(requestedSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    return image;
}
