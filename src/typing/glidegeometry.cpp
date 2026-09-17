#include "glidegeometry.h"

#include <algorithm>
#include <cmath>

namespace GlideGeometry
{
namespace
{
constexpr double MaxSquaredMiss = 4.0;

double squaredMiss(const QPointF &a, const QPointF &b)
{
    const double d = distance(a, b);
    return std::min(d * d, MaxSquaredMiss);
}
}

double distance(const QPointF &a, const QPointF &b)
{
    return std::hypot(a.x() - b.x(), a.y() - b.y());
}

double pathLength(const QPointF *points, int count)
{
    double total = 0;
    for (int i = 1; i < count; ++i) {
        total += distance(points[i], points[i - 1]);
    }
    return total;
}

void resample(const QPointF *points, int count, QPointF *out, int samples)
{
    const double total = pathLength(points, count);
    if (count == 1 || total <= 1e-9) {
        std::fill(out, out + samples, points[0]);
        return;
    }
    const double step = total / (samples - 1);
    out[0] = points[0];
    int written = 1;
    double carried = 0;
    for (int i = 1; i < count && written < samples; ++i) {
        QPointF from = points[i - 1];
        double segment = distance(points[i], from);
        while (carried + segment >= step && written < samples) {
            from += (points[i] - from) * ((step - carried) / segment);
            out[written++] = from;
            segment = distance(points[i], from);
            carried = 0;
        }
        carried += segment;
    }
    std::fill(out + written, out + samples, points[count - 1]);
}

void normalizeShape(const QPointF *in, QPointF *out, int count)
{
    double minX = in[0].x();
    double maxX = minX;
    double minY = in[0].y();
    double maxY = minY;
    QPointF centroid;
    for (int i = 0; i < count; ++i) {
        minX = std::min(minX, in[i].x());
        maxX = std::max(maxX, in[i].x());
        minY = std::min(minY, in[i].y());
        maxY = std::max(maxY, in[i].y());
        centroid += in[i];
    }
    centroid /= count;
    const double scale = 1.0 / std::max({maxX - minX, maxY - minY, 1.0});
    for (int i = 0; i < count; ++i) {
        out[i] = (in[i] - centroid) * scale;
    }
}

double meanDistance(const QPointF *a, const QPointF *b, int count)
{
    double total = 0;
    for (int i = 0; i < count; ++i) {
        total += distance(a[i], b[i]);
    }
    return total / count;
}

double keyAlignmentCost(const FineSamples &gesture, const QPointF *centres, int count)
{
    std::array<double, FineSampleCount> rowA;
    std::array<double, FineSampleCount> rowB;
    double *previous = rowA.data();
    double *current = rowB.data();
    double best = MaxSquaredMiss;
    for (int i = 0; i < FineSampleCount; ++i) {
        best = std::min(best, squaredMiss(gesture[i], centres[0]));
        previous[i] = best;
    }
    for (int k = 1; k < count; ++k) {
        best = 1e9;
        for (int i = 0; i < FineSampleCount; ++i) {
            best = std::min(best, squaredMiss(gesture[i], centres[k]) + previous[i]);
            current[i] = best;
        }
        std::swap(previous, current);
    }
    return previous[FineSampleCount - 1] / count;
}

double segmentDistance(const QPointF &point, const QPointF &a, const QPointF &b)
{
    const QPointF ab = b - a;
    const double lengthSquared = QPointF::dotProduct(ab, ab);
    if (lengthSquared <= 1e-12) {
        return distance(point, a);
    }
    const double t = std::clamp(QPointF::dotProduct(point - a, ab) / lengthSquared, 0.0, 1.0);
    return distance(point, a + ab * t);
}

double coverageCost(const FineSamples &gesture, const QPointF *centres, int count)
{
    double total = 0;
    for (const QPointF &point : gesture) {
        double nearest = distance(point, centres[0]);
        for (int k = 1; k < count; ++k) {
            nearest = std::min(nearest, segmentDistance(point, centres[k - 1], centres[k]));
        }
        total += std::min(nearest * nearest, MaxSquaredMiss);
    }
    return total / FineSampleCount;
}
}
