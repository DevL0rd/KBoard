#include "clipboardlatest.h"

namespace
{
constexpr int s_freshMs = 20000;
}

ClipboardLatest::ClipboardLatest(QObject *parent)
    : QObject(parent)
{
    m_timer.setSingleShot(true);
    m_timer.setInterval(s_freshMs);
    connect(&m_timer, &QTimer::timeout, this, [this] { setFresh(false); });
}

QString ClipboardLatest::id() const
{
    return m_id;
}

QVariantMap ClipboardLatest::value() const
{
    return m_value;
}

bool ClipboardLatest::isFresh() const
{
    return m_fresh;
}

void ClipboardLatest::announce(const QString &id, const QVariantMap &value)
{
    m_id = id;
    m_value = value;
    Q_EMIT valueChanged();
    m_timer.start();
    setFresh(true);
}

void ClipboardLatest::refresh(const QString &id, const QVariantMap &value)
{
    if (id != m_id) {
        return;
    }
    m_value = value;
    Q_EMIT valueChanged();
}

void ClipboardLatest::forget(const QString &id)
{
    if (id != m_id) {
        return;
    }
    m_id.clear();
    m_value.clear();
    Q_EMIT valueChanged();
    dismiss();
}

void ClipboardLatest::dismiss()
{
    m_timer.stop();
    setFresh(false);
}

void ClipboardLatest::setFresh(bool fresh)
{
    if (m_fresh == fresh) {
        return;
    }
    m_fresh = fresh;
    Q_EMIT freshChanged();
}
