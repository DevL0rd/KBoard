#pragma once

#include "keyentry.h"

#include <QList>
#include <QVariantList>

namespace KeyLayout
{
struct Params
{
    qreal width = 0;
    qreal height = 0;
    qreal gap = 0;
    bool split = false;
    qreal splitGap = 0;
};

struct SplitPoint
{
    int index = 0;
    qreal leftPart = 0;
};

QList<KeyEntry> compute(const QVariantMap &page, const Params &params);
SplitPoint splitPoint(const QVariantList &keys);
int keyAt(const QList<KeyEntry> &entries, QPointF point);
int neighbor(const QList<KeyEntry> &entries, int index, int dx, int dy);
QVariantList halfRects(const Params &params);
qreal halfWidth(const Params &params);
}
