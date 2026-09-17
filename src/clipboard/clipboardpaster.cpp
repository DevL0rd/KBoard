#include "clipboardpaster.h"
#include "clipboardimagestore.h"
#include "systemclipboard.h"

#include "inputcontext.h"

#include <KLocalizedString>

ClipboardPaster::ClipboardPaster(const ClipboardImageStore &images)
    : m_images(images)
{ }

void ClipboardPaster::setSystemClipboard(SystemClipboard *clipboard)
{
    m_clipboard = clipboard;
}

QString ClipboardPaster::errorString() const
{
    return m_errorString;
}

bool ClipboardPaster::fail(const QString &error)
{
    m_errorString = error;
    return false;
}

bool ClipboardPaster::paste(const ClipboardEntry &entry)
{
    auto context = InputContext::instance();
    if (!context->isActive()) {
        return fail(i18n("Nothing to paste into: no text field is focused."));
    }
    if (!entry.image) {
        context->commit(entry.otpCode.isEmpty() ? entry.text : entry.otpCode);
    } else if (!copy(entry)) {
        return false;
    } else if (!context->shortcut(QStringLiteral("ctrl+v"))) {
        return fail(i18n("Could not send Ctrl+V to the focused app."));
    }
    m_errorString.clear();
    return true;
}

bool ClipboardPaster::copy(const ClipboardEntry &entry)
{
    if (!m_clipboard) {
        return fail(i18n("The system clipboard is not being watched by this KBoard instance."));
    }
    if (!entry.image) {
        return m_clipboard->publishText(entry.text) || fail(m_clipboard->errorString());
    }
    const QImage full = m_images.image(entry.id, false);
    if (full.isNull()) {
        return fail(i18n("This image is no longer available."));
    }
    return m_clipboard->publishImage(full) || fail(m_clipboard->errorString());
}
