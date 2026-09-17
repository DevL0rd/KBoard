#include "clipboardexpiry.h"

#include "kboardsettings.h"

#include <algorithm>
#include <limits>

namespace
{
constexpr qint64 s_expiringLeadMs = 60000;

qint64 expirySpanMs()
{
    return qint64(KBoardSettings::clipboardExpiryMinutes()) * 60000;
}
}

ClipboardExpiry::ClipboardExpiry(QObject *parent)
    : QObject(parent)
    , m_clock([] { return QDateTime::currentDateTimeUtc(); })
{
    m_timer.setSingleShot(true);
    m_timer.setTimerType(Qt::CoarseTimer);
    connect(&m_timer, &QTimer::timeout, this, &ClipboardExpiry::due);
}

void ClipboardExpiry::setClock(const Clock &clock)
{
    m_clock = clock;
}

QDateTime ClipboardExpiry::now() const
{
    return m_clock();
}

QDateTime ClipboardExpiry::expiryFor(const ClipboardEntry &entry) const
{
    if (entry.pinned || expirySpanMs() <= 0) {
        return {};
    }
    return entry.timestamp.addMSecs(expirySpanMs());
}

QDateTime ClipboardExpiry::expiringFor(const ClipboardEntry &entry) const
{
    const QDateTime expiry = expiryFor(entry);
    return expiry.isValid() ? expiry.addMSecs(-std::min(s_expiringLeadMs, expirySpanMs() / 10)) : QDateTime();
}

ClipboardExpiry::Sweep ClipboardExpiry::sweep(const QList<ClipboardEntry> &entries) const
{
    const QDateTime current = now();
    Sweep result;
    for (int row = 0; row < entries.size(); ++row) {
        const ClipboardEntry &entry = entries.at(row);
        const QDateTime expiry = expiryFor(entry);
        if (!expiry.isValid()) {
            continue;
        }
        if (expiry <= current) {
            result.expired.append(row);
        } else if (!entry.expiring && expiringFor(entry) <= current) {
            result.expiring.append(row);
        }
    }
    return result;
}

void ClipboardExpiry::schedule(const QList<ClipboardEntry> &entries)
{
    QDateTime next;
    for (const ClipboardEntry &entry : entries) {
        const QDateTime candidate = entry.expiring ? expiryFor(entry) : expiringFor(entry);
        if (candidate.isValid() && (!next.isValid() || candidate < next)) {
            next = candidate;
        }
    }
    m_next = next;
    if (!next.isValid()) {
        m_timer.stop();
        return;
    }
    const qint64 delay = std::clamp<qint64>(now().msecsTo(next), 0, std::numeric_limits<int>::max());
    m_timer.start(int(delay));
}

QDateTime ClipboardExpiry::nextEventTime() const
{
    return m_next;
}

bool ClipboardExpiry::isActive() const
{
    return m_timer.isActive();
}

int ClipboardExpiry::interval() const
{
    return m_timer.interval();
}
