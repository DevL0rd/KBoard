#pragma once

#include "clipboardentry.h"

#include <QString>

class ClipboardImageStore;
class SystemClipboard;

class ClipboardPaster
{
public:
    explicit ClipboardPaster(const ClipboardImageStore &images);

    void setSystemClipboard(SystemClipboard *clipboard);
    QString errorString() const;
    bool paste(const ClipboardEntry &entry);
    bool copy(const ClipboardEntry &entry);

private:
    bool fail(const QString &error);

    const ClipboardImageStore &m_images;
    SystemClipboard *m_clipboard = nullptr;
    QString m_errorString;
};
