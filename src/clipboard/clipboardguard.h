#pragma once

#include <QByteArray>
#include <QSet>
#include <QString>

#include <functional>

class ClipboardGuard
{
public:
    using SensitiveProbe = std::function<bool()>;

    void setSensitiveProbe(const SensitiveProbe &probe);
    static bool isRecording();
    bool allowsCopy(const QString &text);
    bool allowsImport(const QString &text) const;
    void block(const QString &text);
    void dismiss(const QString &text);
    void forgive(const QString &text);

private:
    static QByteArray hash(const QString &text);

    SensitiveProbe m_probe;
    QSet<QByteArray> m_blocked;
    QSet<QByteArray> m_dismissed;
};
