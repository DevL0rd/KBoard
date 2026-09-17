#pragma once

#include <QPointer>
#include <QQuickImageProvider>

class ClipboardHistory;

class ClipboardImageProvider : public QQuickImageProvider
{
public:
    explicit ClipboardImageProvider(ClipboardHistory *history);

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;

private:
    QPointer<ClipboardHistory> m_history;
};
