#include "typingengine.h"

#include "kboardsettings.h"

#include <QPointF>

namespace
{
QPointF toPoint(const QVariant &value)
{
    if (value.typeId() == QMetaType::QPointF || value.typeId() == QMetaType::QPoint) {
        return value.toPointF();
    }
    const QVariantMap map = value.toMap();
    return QPointF(map.value(QStringLiteral("x")).toDouble(), map.value(QStringLiteral("y")).toDouble());
}
}

GlideQuery TypingEngine::glideQuery(const QVariantList &points) const
{
    GlideQuery query;
    query.points.reserve(points.size());
    for (const QVariant &value : points) {
        query.points.append(toPoint(value));
    }
    query.previousKeys = m_context.previousKeys;
    query.uppercase = m_policy.forcesUppercase();
    query.capitalise = m_shouldCapitalize && !query.uppercase;
    return query;
}

QStringList TypingEngine::decodeGlide(const QVariantList &points)
{
    if (!KBoardSettings::self()->glideTyping() || !m_policy.allowsSuggestions()) {
        return {};
    }
    return m_glide.decode(m_data, m_store.model(), glideQuery(points));
}

void TypingEngine::decodeGlideAsync(const QVariantList &points)
{
    GlideQuery query = glideQuery(points);
    if (!KBoardSettings::self()->glideTyping() || !m_policy.allowsSuggestions()) {
        query.points.clear();
    }
    m_glide.decodeAsync(m_data, m_store.model(), query);
}
