#include "clipboardguard.h"
#include "clipboarddetect.h"

#include "kboardsettings.h"

#include <QCryptographicHash>

void ClipboardGuard::setSensitiveProbe(const SensitiveProbe &probe)
{
    m_probe = probe;
}

QByteArray ClipboardGuard::hash(const QString &text)
{
    return QCryptographicHash::hash(text.toUtf8(), QCryptographicHash::Sha256);
}

bool ClipboardGuard::isRecording()
{
    return KBoardSettings::clipboardEnabled() && !KBoardSettings::clipboardPaused();
}

bool ClipboardGuard::allowsCopy(const QString &text)
{
    if (!isRecording()) {
        return false;
    }
    if (m_probe && m_probe()) {
        block(text);
        return false;
    }
    return text.isEmpty() || !m_blocked.contains(hash(text));
}

bool ClipboardGuard::allowsImport(const QString &text) const
{
    if (!isRecording() || !KBoardSettings::clipboardImportKlipper() || text.trimmed().isEmpty()) {
        return false;
    }
    const QByteArray key = hash(text);
    return !ClipboardDetect::isKlipperImagePlaceholder(text) && !m_blocked.contains(key) && !m_dismissed.contains(key);
}

void ClipboardGuard::block(const QString &text)
{
    if (!text.isEmpty()) {
        m_blocked.insert(hash(text));
    }
}

void ClipboardGuard::dismiss(const QString &text)
{
    m_dismissed.insert(hash(text));
}

void ClipboardGuard::forgive(const QString &text)
{
    m_dismissed.remove(hash(text));
}
