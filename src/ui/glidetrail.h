#pragma once

#include "trailsection.h"

#include <QColor>
#include <QElapsedTimer>
#include <QList>
#include <QQuickItem>

class GlideTrail : public QQuickItem
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY appearanceChanged)
    Q_PROPERTY(qreal lineWidth READ lineWidth WRITE setLineWidth NOTIFY appearanceChanged)
    Q_PROPERTY(qreal glowWidth READ glowWidth WRITE setGlowWidth NOTIFY appearanceChanged)
    Q_PROPERTY(int tailDuration READ tailDuration WRITE setTailDuration NOTIFY appearanceChanged)
    Q_PROPERTY(int fadeDuration READ fadeDuration WRITE setFadeDuration NOTIFY appearanceChanged)
    Q_PROPERTY(bool active READ isActive NOTIFY activeChanged)
    Q_PROPERTY(int pointCount READ pointCount NOTIFY pointCountChanged)

public:
    explicit GlideTrail(QQuickItem *parent = nullptr);

    QColor color() const;
    void setColor(const QColor &color);
    qreal lineWidth() const;
    void setLineWidth(qreal width);
    qreal glowWidth() const;
    void setGlowWidth(qreal width);
    int tailDuration() const;
    void setTailDuration(int duration);
    int fadeDuration() const;
    void setFadeDuration(int duration);
    bool isActive() const;
    int pointCount() const;

    Q_INVOKABLE void begin(qreal x, qreal y);
    Q_INVOKABLE void addPoint(qreal x, qreal y);
    Q_INVOKABLE void finish();
    Q_INVOKABLE void clear();

Q_SIGNALS:
    void appearanceChanged();
    void activeChanged();
    void pointCountChanged();

protected:
    qreal fadeAt(qint64 now) const;
    QList<Section> sections(qint64 now) const;
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *data) override;

private:
    struct Point
    {
        QPointF position;
        qint64 time;
    };

    QColor m_color = QColor(61, 174, 233);
    qreal m_lineWidth = 6;
    qreal m_glowWidth = 18;
    int m_tailDuration = 600;
    int m_fadeDuration = 350;
    bool m_active = false;
    qint64 m_finishedAt = -1;
    QList<TrailPoint> m_points;
    QElapsedTimer m_clock;
};
