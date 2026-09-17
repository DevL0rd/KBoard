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

    QDateTime nextEventTime() const;
    bool isActive() const;
    int interval() const;

Q_SIGNALS:
    void due();

private:
    Clock m_clock;
    QTimer m_timer;
    QDateTime m_next;
};
