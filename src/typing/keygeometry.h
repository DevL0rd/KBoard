#pragma once

#include <QHash>
#include <QPointF>
#include <QString>
#include <QVariantList>
#include <QVector>

class KeyGeometry
{
public:
    struct Key
    {
        QChar letter;
        QPointF centre;
        double width = 1.0;
        double height = 1.0;
    };

    static KeyGeometry fromVariantList(const QVariantList &keys);
    static KeyGeometry qwerty();

    bool isEmpty() const;
    int keyCount() const;
    int indexOf(QChar letter) const;
    double unit() const;
    double distance(QChar a, QChar b) const;
    QPointF normalizedCentre(int index) const;
    quint64 fingerprint() const;

private:
    void finish();

    QVector<Key> m_keys;
    QHash<QChar, int> m_index;
    double m_unit = 1.0;
    quint64 m_fingerprint = 0;
};
