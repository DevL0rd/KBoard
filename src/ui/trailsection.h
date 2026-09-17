#pragma once

#include <QPointF>

struct TrailPoint
{
    QPointF position;
    qint64 time;
};

struct Section
{
    QPointF center;
    QPointF normal;
    qreal life;
};
