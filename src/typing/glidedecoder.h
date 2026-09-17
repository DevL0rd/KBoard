#pragma once

#include <QPointF>
#include <QString>
#include <QVector>

#include <functional>

class KeyGeometry;

class GlideIndex
{
public:
    void build(const QVector<QString> &words, const KeyGeometry &geometry);

    int keyCount() const { return m_keyCount; }
    const QVector<int> &bucket(int firstKey, int lastKey) const { return m_buckets[firstKey * m_keyCount + lastKey]; }
    int sequenceLength(int word) const { return m_lengths[word]; }
    const quint8 *sequence(int word) const { return m_sequence.constData() + m_offsets[word]; }
    float pathLength(int word) const { return m_pathLengths[word]; }

private:
    void append(const QVector<quint8> &keys, const KeyGeometry &geometry);

    int m_keyCount = 0;
    QVector<quint8> m_sequence;
    QVector<int> m_offsets;
    QVector<quint8> m_lengths;
    QVector<float> m_pathLengths;
    QVector<QVector<int>> m_buckets;
};

class GlideDecoder
{
public:
    struct Candidate
    {
        int word = -1;
        double score = 0;
    };
    using Prior = std::function<double(int)>;

    explicit GlideDecoder(const KeyGeometry &geometry);

    QVector<Candidate> decode(const GlideIndex &index, const QVector<QPointF> &points, const Prior &prior, int limit) const;

private:
    struct Gesture;
    struct Shortlisted
    {
        int word;
        double location;
    };

    Gesture prepare(const QVector<QPointF> &points) const;
    QVector<Shortlisted> shortlist(const GlideIndex &index, const Gesture &gesture) const;
    void scanBucket(const GlideIndex &index, const QVector<int> &bucket, const Gesture &gesture, QVector<Shortlisted> &out) const;
    double score(const GlideIndex &index, const Shortlisted &item, const Gesture &gesture) const;
    int loadCentres(const GlideIndex &index, int word, QPointF *centres) const;

    const KeyGeometry &m_geometry;
};
