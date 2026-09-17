#include "systemclipboard.h"

#include <KLocalizedString>
#include <KSystemClipboard>

#include <QGuiApplication>
#include <QImage>
#include <QMimeData>

SystemClipboard::SystemClipboard(QObject *parent)
    : QObject(parent)
{ }

bool SystemClipboard::start()
{
    auto clipboard = KSystemClipboard::instance();
    if (!clipboard) {
        m_errorString = i18n("The system clipboard is not available.");
        return false;
    }
    const QByteArrayView className(clipboard->metaObject()->className());
    const bool focusless = className == "WaylandClipboard" || className == "WlrWaylandClipboard";
    if (qGuiApp->platformName().startsWith(QLatin1String("wayland")) && !focusless) {
        m_errorString = i18n("The compositor does not offer ext_data_control_v1, so KBoard cannot see copies made in other apps.");
        return false;
    }
    connect(clipboard, &KSystemClipboard::changed, this, [this, clipboard](QClipboard::Mode mode) {
        if (mode == QClipboard::Clipboard && !clipboard->ownsClipboard()) {
            Q_EMIT copied(clipboard->mimeData(QClipboard::Clipboard));
        }
    });
    return true;
}

QString SystemClipboard::errorString() const
{
    return m_errorString;
}

bool SystemClipboard::publishText(const QString &text)
{
    auto mime = new QMimeData;
    mime->setText(text);
    return publish(mime);
}

bool SystemClipboard::publishImage(const QImage &image)
{
    auto mime = new QMimeData;
    mime->setImageData(image);
    return publish(mime);
}

bool SystemClipboard::publish(QMimeData *mime)
{
    auto clipboard = KSystemClipboard::instance();
    if (!clipboard) {
        delete mime;
        m_errorString = i18n("The system clipboard is not available.");
        return false;
    }
    clipboard->setMimeData(mime, QClipboard::Clipboard);
    return true;
}
