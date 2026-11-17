#pragma once

#include "clipboardentry.h"

#include <QObject>
#include <QTimer>

#include <functional>

class ClipboardExpiry : public QObject
{
    Q_OBJECT

public:
    using Clock = std::function<QDateTime()>;

    struct Sweep
    {
        QList<int> expired;
        QList<int> expiring;
    };

    explicit ClipboardExpiry(QObject *parent = nullptr);

    void setClock(const Clock &clock);
    QDateTime now() const;
    QDateTime expiryFor(const ClipboardEntry &entry) const;
    QDateTime expiringFor(const ClipboardEntry &entry) const;
    Sweep sweep(const QList<ClipboardEntry> &entries) const;
    void schedule(const QList<ClipboardEntry> &entries);

Q_SIGNALS:
    void due();

private:
    friend struct ClipboardExpiryProbe;

    Clock m_clock;
    QTimer m_timer;
};
