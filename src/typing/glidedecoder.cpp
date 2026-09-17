#include "glidedecoder.h"

#include "glidegeometry.h"
#include "keygeometry.h"

#include <QVarLengthArray>

#include <algorithm>
#include <array>

using namespace GlideGeometry;

namespace
{
constexpr int ShortlistSize = 400;
constexpr double LocationSigma = 0.55;
constexpr double ShapeSigma = 0.22;
constexpr double KeySigma = 0.6;
constexpr double CoverageSigma = 0.25;
constexpr double PriorWeight = 0.3;
constexpr double MaxLocation = 2.2;
constexpr double EndpointRadius = 1.25;
constexpr double EndpointSlack = 0.6;
constexpr int MaxSequence = 255;

double gaussianPenalty(double value, double sigma)
{
    return (value * value) / (2 * sigma * sigma);
}
}

struct GlideDecoder::Gesture
{
    Samples samples;
    Samples shape;
    FineSamples fine;
    QPointF first;
    QPointF last;
    double length = 0;
    QVarLengthArray<bool, 64> startKeys;
    QVarLengthArray<bool, 64> endKeys;
};

void GlideIndex::build(const QVector<QString> &words, const KeyGeometry &geometry)
{
    m_keyCount = geometry.keyCount();
    m_sequence.clear();
    m_offsets.clear();
    m_lengths.clear();
    m_pathLengths.clear();
    m_buckets.clear();
    m_buckets.resize(qsizetype(m_keyCount) * m_keyCount);
    m_sequence.reserve(words.size() * 7);
    QVector<quint8> keys;
    for (const QString &word : words) {
        keys.clear();
        for (QChar c : word) {
            if (c == u'\'' || c == u'-') {
                continue;
            }
            const int key = geometry.indexOf(c);
            if (key < 0 || keys.size() >= MaxSequence) {
                keys.clear();
                break;
            }
            if (keys.isEmpty() || keys.last() != key) {
                keys.append(quint8(key));
            }
        }
        append(keys, geometry);
    }
}

void GlideIndex::append(const QVector<quint8> &keys, const KeyGeometry &geometry)
{
    const int word = m_offsets.size();
    m_offsets.append(m_sequence.size());
    m_lengths.append(quint8(keys.size()));
    float path = 0;
    for (int i = 0; i < keys.size(); ++i) {
        m_sequence.append(keys[i]);
        if (i > 0) {
            path += float(distance(geometry.normalizedCentre(keys[i]), geometry.normalizedCentre(keys[i - 1])));
        }
    }
    m_pathLengths.append(path);
    if (!keys.isEmpty()) {
        m_buckets[keys.first() * m_keyCount + keys.last()].append(word);
    }
}

GlideDecoder::GlideDecoder(const KeyGeometry &geometry)
    : m_geometry(geometry)
{ }

GlideDecoder::Gesture GlideDecoder::prepare(const QVector<QPointF> &rawPoints) const
{
    QVector<QPointF> points;
    points.reserve(rawPoints.size());
    for (const QPointF &p : rawPoints) {
        const QPointF scaled = p / m_geometry.unit();
        if (points.isEmpty() || distance(scaled, points.last()) > 0.02) {
            points.append(scaled);
        }
    }
    Gesture gesture;
    gesture.first = points.first();
    gesture.last = points.last();
    gesture.length = pathLength(points.constData(), points.size());
    resample(points.constData(), points.size(), gesture.samples.data(), SampleCount);
    normalizeShape(gesture.samples.data(), gesture.shape.data(), SampleCount);
    resample(points.constData(), points.size(), gesture.fine.data(), FineSampleCount);

    const int keyCount = m_geometry.keyCount();
    QVarLengthArray<double, 64> start(keyCount);
    QVarLengthArray<double, 64> end(keyCount);
    for (int k = 0; k < keyCount; ++k) {
        start[k] = distance(m_geometry.normalizedCentre(k), gesture.first);
        end[k] = distance(m_geometry.normalizedCentre(k), gesture.last);
    }
    const double startRadius = std::max(EndpointRadius, *std::min_element(start.cbegin(), start.cend()) + EndpointSlack);
    const double endRadius = std::max(EndpointRadius, *std::min_element(end.cbegin(), end.cend()) + EndpointSlack);
    gesture.startKeys.resize(keyCount);
    gesture.endKeys.resize(keyCount);
    for (int k = 0; k < keyCount; ++k) {
        gesture.startKeys[k] = start[k] <= startRadius;
        gesture.endKeys[k] = end[k] <= endRadius;
    }
    return gesture;
}

int GlideDecoder::loadCentres(const GlideIndex &index, int word, QPointF *centres) const
{
    const int count = index.sequenceLength(word);
    const quint8 *keys = index.sequence(word);
    for (int i = 0; i < count; ++i) {
        centres[i] = m_geometry.normalizedCentre(keys[i]);
    }
    return count;
}

void GlideDecoder::scanBucket(const GlideIndex &index, const QVector<int> &bucket, const Gesture &gesture, QVector<Shortlisted> &out) const
{
    std::array<QPointF, MaxSequence> centres;
    Samples templ;
    const double longest = gesture.length * 1.6 + 1.5;
    const double shortest = gesture.length * 0.55 - 1.5;
    for (int word : bucket) {
        const double templateLength = index.pathLength(word);
        if (templateLength > longest || templateLength < shortest) {
            continue;
        }
        const int count = loadCentres(index, word, centres.data());
        resample(centres.data(), count, templ.data(), SampleCount);
        const double location = meanDistance(templ.data(), gesture.samples.data(), SampleCount);
        if (location <= MaxLocation) {
            out.append(Shortlisted {word, location});
        }
    }
}

QVector<GlideDecoder::Shortlisted> GlideDecoder::shortlist(const GlideIndex &index, const Gesture &gesture) const
{
    QVector<Shortlisted> items;
    items.reserve(4096);
    const int keyCount = m_geometry.keyCount();
    for (int s = 0; s < keyCount; ++s) {
        for (int e = 0; gesture.startKeys[s] && e < keyCount; ++e) {
            if (gesture.endKeys[e]) {
                scanBucket(index, index.bucket(s, e), gesture, items);
            }
        }
    }
    if (items.size() > ShortlistSize) {
        std::partial_sort(items.begin(), items.begin() + ShortlistSize, items.end(),
            [](const Shortlisted &a, const Shortlisted &b) { return a.location < b.location; });
        items.resize(ShortlistSize);
    }
    return items;
}

double GlideDecoder::score(const GlideIndex &index, const Shortlisted &item, const Gesture &gesture) const
{
    std::array<QPointF, MaxSequence> centres;
    const int count = loadCentres(index, item.word, centres.data());
    Samples templ;
    Samples templShape;
    resample(centres.data(), count, templ.data(), SampleCount);
    normalizeShape(templ.data(), templShape.data(), SampleCount);
    const double shape = meanDistance(templShape.data(), gesture.shape.data(), SampleCount);
    const double keyMiss = keyAlignmentCost(gesture.fine, centres.data(), count);
    const double coverage = coverageCost(gesture.fine, centres.data(), count);
    return -gaussianPenalty(item.location, LocationSigma) - gaussianPenalty(shape, ShapeSigma) - keyMiss / (2 * KeySigma * KeySigma)
        - coverage / (2 * CoverageSigma * CoverageSigma);
}

QVector<GlideDecoder::Candidate> GlideDecoder::decode(
    const GlideIndex &index, const QVector<QPointF> &points, const Prior &prior, int limit) const
{
    QVector<Candidate> result;
    if (points.isEmpty() || index.keyCount() == 0 || index.keyCount() != m_geometry.keyCount()) {
        return result;
    }
    const Gesture gesture = prepare(points);
    const QVector<Shortlisted> items = shortlist(index, gesture);
    result.reserve(items.size());
    for (const Shortlisted &item : items) {
        result.append(Candidate {item.word, score(index, item, gesture) + PriorWeight * prior(item.word)});
    }
    std::sort(result.begin(), result.end(), [](const Candidate &a, const Candidate &b) { return a.score > b.score; });
    if (result.size() > limit) {
        result.resize(limit);
    }
    return result;
}
