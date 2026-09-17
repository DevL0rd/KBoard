#pragma once

#include <QPointF>

#include <array>

namespace GlideGeometry
{
constexpr int SampleCount = 32;
constexpr int FineSampleCount = 64;

using Samples = std::array<QPointF, SampleCount>;
using FineSamples = std::array<QPointF, FineSampleCount>;

double distance(const QPointF &a, const QPointF &b);
double pathLength(const QPointF *points, int count);
void resample(const QPointF *points, int count, QPointF *out, int samples);
void normalizeShape(const QPointF *in, QPointF *out, int count);
double meanDistance(const QPointF *a, const QPointF *b, int count);
double keyAlignmentCost(const FineSamples &gesture, const QPointF *centres, int count);
double coverageCost(const FineSamples &gesture, const QPointF *centres, int count);
}
