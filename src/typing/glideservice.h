#pragma once

#include "keygeometry.h"
#include "usermodel.h"

#include <QFutureWatcher>
#include <QObject>
#include <QPointF>
#include <QStringList>

#include <memory>

class GlideIndex;
struct LanguageData;

struct GlideQuery
{
    QVector<QPointF> points;
    QStringList previousKeys;
    bool capitalise = false;
    bool uppercase = false;
};

class GlideService : public QObject
{
    Q_OBJECT

public:
    explicit GlideService(QObject *parent = nullptr);
    ~GlideService() override;

    void rebuild(const std::shared_ptr<const LanguageData> &data, const KeyGeometry &geometry);
    void reset();
    bool isReady() const;

    QStringList decode(const std::shared_ptr<const LanguageData> &data, const UserModel &user, const GlideQuery &query);
    void decodeAsync(const std::shared_ptr<const LanguageData> &data, const UserModel &user, const GlideQuery &query);

    static QStringList search(
        const LanguageData &data, const UserModel &user, const GlideIndex &index, const KeyGeometry &geometry, const GlideQuery &query);

Q_SIGNALS:
    void readyChanged();
    void decoded(const QStringList &candidates);

private:
    void adoptBuiltIndex();
    void ensureIndex();
    void setIndex(const std::shared_ptr<GlideIndex> &index);

    std::shared_ptr<GlideIndex> m_index;
    KeyGeometry m_geometry;
    quint64 m_pendingFingerprint = 0;
    QFutureWatcher<std::shared_ptr<GlideIndex>> m_builder;
    int m_request = 0;
};
