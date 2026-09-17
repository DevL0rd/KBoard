#include "glidetrail.h"

#include <QSGGeometryNode>
#include <QSGVertexColorMaterial>

#include <cmath>

namespace
{
QSGGeometryNode *ensureNode(QSGNode *parent, int index)
{
    QSGNode *child = parent->childAtIndex(index);
    if (child) {
        return static_cast<QSGGeometryNode *>(child);
    }
    auto *node = new QSGGeometryNode;
    auto *geometry = new QSGGeometry(QSGGeometry::defaultAttributes_ColoredPoint2D(), 0);
    geometry->setDrawingMode(QSGGeometry::DrawTriangles);
    node->setGeometry(geometry);
    node->setFlag(QSGNode::OwnsGeometry);
    auto *material = new QSGVertexColorMaterial;
    node->setMaterial(material);
    node->setFlag(QSGNode::OwnsMaterial);
    parent->appendChildNode(node);
    return node;
}

struct Strip
{
    QList<qreal> offsets;
    QList<qreal> alphas;
    qreal halfWidth;
    QColor color;
    qreal fade;
};

struct Corner
{
    QPointF point;
    qreal alpha;
};

void setVertex(QSGGeometry::ColoredPoint2D *vertex, const Corner &corner, const QColor &color)
{
    const qreal a = qBound<qreal>(0, corner.alpha, 1);
    vertex->set(float(corner.point.x()), float(corner.point.y()), uchar(color.red() * a), uchar(color.green() * a), uchar(color.blue() * a),
        uchar(255 * a));
}

Corner cornerAt(const Section &section, const Strip &strip, int column)
{
    const qreal width = strip.halfWidth * (0.3 + 0.7 * section.life);
    return {section.center + section.normal * (strip.offsets.at(column) * width), strip.alphas.at(column) * section.life * strip.fade};
}

void appendQuad(QSGGeometry::ColoredPoint2D *vertices, int &index, const Strip &strip, const Section &from, const Section &to, int column)
{
    const Corner a = cornerAt(from, strip, column);
    const Corner b = cornerAt(from, strip, column + 1);
    const Corner c = cornerAt(to, strip, column);
    const Corner d = cornerAt(to, strip, column + 1);
    for (const Corner &corner : {a, b, c, b, d, c}) {
        setVertex(&vertices[index++], corner, strip.color);
    }
}

void buildStrip(QSGGeometryNode *node, const QList<Section> &sections, const Strip &strip)
{
    QSGGeometry *geometry = node->geometry();
    const int columns = int(strip.offsets.size());
    const int quads = sections.size() < 2 ? 0 : (columns - 1) * int(sections.size() - 1);
    geometry->allocate(quads * 6);
    auto *vertices = geometry->vertexDataAsColoredPoint2D();
    int index = 0;
    for (int s = 0; s + 1 < sections.size(); ++s) {
        for (int column = 0; column + 1 < columns; ++column) {
            appendQuad(vertices, index, strip, sections.at(s), sections.at(s + 1), column);
        }
    }
    node->markDirty(QSGNode::DirtyGeometry);
}
}

GlideTrail::GlideTrail(QQuickItem *parent)
    : QQuickItem(parent)
{
    setFlag(ItemHasContents, true);
    m_clock.start();
}

QColor GlideTrail::color() const
{
    return m_color;
}

void GlideTrail::setColor(const QColor &color)
{
    if (color == m_color) {
        return;
    }
    m_color = color;
    update();
    Q_EMIT appearanceChanged();
}

qreal GlideTrail::lineWidth() const
{
    return m_lineWidth;
}

void GlideTrail::setLineWidth(qreal width)
{
    if (qFuzzyCompare(width, m_lineWidth)) {
        return;
    }
    m_lineWidth = width;
    update();
    Q_EMIT appearanceChanged();
}

qreal GlideTrail::glowWidth() const
{
    return m_glowWidth;
}

void GlideTrail::setGlowWidth(qreal width)
{
    if (qFuzzyCompare(width, m_glowWidth)) {
        return;
    }
    m_glowWidth = width;
    update();
    Q_EMIT appearanceChanged();
}

int GlideTrail::tailDuration() const
{
    return m_tailDuration;
}

void GlideTrail::setTailDuration(int duration)
{
    if (duration == m_tailDuration) {
        return;
    }
    m_tailDuration = duration;
    Q_EMIT appearanceChanged();
}

int GlideTrail::fadeDuration() const
{
    return m_fadeDuration;
}

void GlideTrail::setFadeDuration(int duration)
{
    if (duration == m_fadeDuration) {
        return;
    }
    m_fadeDuration = duration;
    Q_EMIT appearanceChanged();
}

bool GlideTrail::isActive() const
{
    return m_active;
}

int GlideTrail::pointCount() const
{
    return int(m_points.size());
}

void GlideTrail::begin(qreal x, qreal y)
{
    m_points.clear();
    m_finishedAt = -1;
    m_points << TrailPoint {QPointF(x, y), m_clock.elapsed()};
    if (!m_active) {
        m_active = true;
        Q_EMIT activeChanged();
    }
    Q_EMIT pointCountChanged();
    update();
}

void GlideTrail::addPoint(qreal x, qreal y)
{
    if (!m_active) {
        begin(x, y);
        return;
    }
    const QPointF point(x, y);
    if (!m_points.isEmpty() && QLineF(m_points.constLast().position, point).length() < 1.5) {
        m_points.last().time = m_clock.elapsed();
        return;
    }
    m_points << TrailPoint {point, m_clock.elapsed()};
    Q_EMIT pointCountChanged();
    update();
}

void GlideTrail::finish()
{
    if (!m_active) {
        return;
    }
    m_active = false;
    m_finishedAt = m_clock.elapsed();
    Q_EMIT activeChanged();
    if (m_fadeDuration <= 0) {
        clear();
        return;
    }
    update();
}

void GlideTrail::clear()
{
    m_points.clear();
    m_finishedAt = -1;
    if (m_active) {
        m_active = false;
        Q_EMIT activeChanged();
    }
    Q_EMIT pointCountChanged();
    update();
}

qreal GlideTrail::fadeAt(qint64 now) const
{
    if (m_finishedAt < 0) {
        return 1.0;
    }
    return m_fadeDuration > 0 ? 1.0 - qreal(now - m_finishedAt) / m_fadeDuration : 0.0;
}

QList<Section> GlideTrail::sections(qint64 now) const
{
    QList<Section> result;
    const qint64 newest = m_finishedAt >= 0 ? m_finishedAt : now;
    for (int i = 0; i < m_points.size(); ++i) {
        const qreal life = m_tailDuration > 0 ? 1.0 - qreal(newest - m_points.at(i).time) / m_tailDuration : 1.0;
        const QPointF previous = m_points.at(qMax(0, i - 1)).position;
        const QPointF next = m_points.at(qMin(int(m_points.size()) - 1, i + 1)).position;
        QPointF direction = next - previous;
        const qreal length = std::hypot(direction.x(), direction.y());
        if (life <= 0 || length < 1e-3) {
            continue;
        }
        direction /= length;
        result.append(Section {m_points.at(i).position, QPointF(-direction.y(), direction.x()), std::pow(qBound<qreal>(0, life, 1), 0.7)});
    }
    return result;
}

QSGNode *GlideTrail::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    QSGNode *root = oldNode ? oldNode : new QSGNode;
    const qint64 now = m_clock.elapsed();
    const qreal fade = qMax<qreal>(0, fadeAt(now));
    const QList<Section> strip = fade > 0 && m_points.size() >= 2 ? sections(now) : QList<Section>();

    const QColor bright = QColor::fromRgbF(m_color.redF() + (1 - m_color.redF()) * 0.45, m_color.greenF() + (1 - m_color.greenF()) * 0.45,
        m_color.blueF() + (1 - m_color.blueF()) * 0.45);
    const qreal feather = 1.2 / qMax<qreal>(1, m_lineWidth / 2);
    buildStrip(ensureNode(root, 0), strip, Strip {{-1.0, 0.0, 1.0}, {0.0, 0.32, 0.0}, m_glowWidth / 2, m_color, fade});
    buildStrip(ensureNode(root, 1), strip,
        Strip {{-(1.0 + feather), -1.0, 1.0, 1.0 + feather}, {0.0, 1.0, 1.0, 0.0}, m_lineWidth / 2, bright, fade});

    if (!strip.isEmpty() && (m_finishedAt < 0 ? m_tailDuration > 0 : fade > 0)) {
        QMetaObject::invokeMethod(this, &QQuickItem::update, Qt::QueuedConnection);
    } else if (m_finishedAt >= 0 && !m_points.isEmpty()) {
        QMetaObject::invokeMethod(this, &GlideTrail::clear, Qt::QueuedConnection);
    }
    return root;
}
