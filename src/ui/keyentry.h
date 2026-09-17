#pragma once

#include <QRectF>
#include <QVariantMap>

struct KeyEntry
{
    QVariantMap key;
    QRectF rect;
    QRectF cell;
    int row = 0;
    int column = 0;
    int half = -1;
    bool pressed = false;
};
