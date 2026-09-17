#include "glideservice.h"

#include "glidedecoder.h"
#include "languagedata.h"
#include "textanalysis.h"
#include "wordscorer.h"

#include <QtConcurrentRun>

#include <algorithm>
#include <cmath>

namespace
{
constexpr int ResultCount = 5;

struct Scored
{
    QString key;
    double score;
};

void collect(QVector<Scored> &out, const QVector<GlideDecoder::Candidate> &candidates, const std::function<QString(int)> &keyOf)
{
    for (const GlideDecoder::Candidate &candidate : candidates) {
        if (std::isfinite(candidate.score)) {
            out.append(Scored {keyOf(candidate.word), candidate.score});
        }
    }
}

QVector<QString> unlistedUserKeys(const LanguageData &data, const UserModel &user)
{
    QVector<QString> keys;
    for (auto it = user.words().cbegin(); it != user.words().cend(); ++it) {
        if (data.lexicon.find(it.key()) < 0 && user.isKnown(it.key()) && TextAnalysis::isPlainWord(it->form)) {
            keys.append(it.key());
        }
    }
    return keys;
}

QString presented(const WordScorer &scorer, const QString &key, const GlideQuery &query)
{
    const QString form = scorer.displayForm(key);
    if (query.uppercase) {
        return form.toUpper();
    }
    return query.capitalise ? TextAnalysis::capitalise(form) : form;
}
}

GlideService::GlideService(QObject *parent)
    : QObject(parent)
{
    connect(&m_builder, &QFutureWatcher<std::shared_ptr<GlideIndex>>::finished, this, &GlideService::adoptBuiltIndex);
}

GlideService::~GlideService() = default;

void GlideService::rebuild(const std::shared_ptr<const LanguageData> &data, const KeyGeometry &geometry)
{
    reset();
    if (!data || geometry.isEmpty()) {
        return;
    }
    m_geometry = geometry;
    m_pendingFingerprint = geometry.fingerprint();
    m_builder.setFuture(QtConcurrent::run([data, geometry] {
        QVector<QString> words;
        words.reserve(data->lexicon.size());
        for (int i = 0; i < data->lexicon.size(); ++i) {
            words.append(data->lexicon.entry(i).key);
        }
        auto index = std::make_shared<GlideIndex>();
        index->build(words, geometry);
        return index;
    }));
}

void GlideService::reset()
{
    m_pendingFingerprint = 0;
    setIndex(nullptr);
}

void GlideService::adoptBuiltIndex()
{
    if (m_pendingFingerprint != 0 && m_pendingFingerprint == m_geometry.fingerprint()) {
        m_pendingFingerprint = 0;
        setIndex(m_builder.result());
    }
}

void GlideService::setIndex(const std::shared_ptr<GlideIndex> &index)
{
    const bool wasReady = isReady();
    m_index = index;
    if (wasReady != isReady()) {
        Q_EMIT readyChanged();
    }
}

void GlideService::ensureIndex()
{
    if (!m_index && m_builder.isRunning()) {
        m_builder.waitForFinished();
        adoptBuiltIndex();
    }
}

bool GlideService::isReady() const
{
    return bool(m_index);
}

QStringList GlideService::search(
    const LanguageData &data, const UserModel &user, const GlideIndex &index, const KeyGeometry &geometry, const GlideQuery &query)
{
    const WordScorer scorer(data, user);
    const GlideDecoder decoder(geometry);
    QVector<Scored> scored;
    const auto lexiconPrior = [&](int word) { return scorer.contextualPrior(query.previousKeys, data.lexicon.entry(word).key, word); };
    collect(
        scored, decoder.decode(index, query.points, lexiconPrior, ResultCount * 2), [&](int word) { return data.lexicon.entry(word).key; });
    const QVector<QString> userKeys = unlistedUserKeys(data, user);
    if (!userKeys.isEmpty()) {
        GlideIndex userIndex;
        userIndex.build(userKeys, geometry);
        const auto userPrior = [&](int word) { return scorer.contextualPrior(query.previousKeys, userKeys[word], -1); };
        collect(scored, decoder.decode(userIndex, query.points, userPrior, ResultCount), [&](int word) { return userKeys[word]; });
    }
    std::sort(scored.begin(), scored.end(), [](const Scored &a, const Scored &b) { return a.score > b.score; });
    QStringList result;
    for (auto it = scored.cbegin(); it != scored.cend() && result.size() < ResultCount; ++it) {
        const QString text = presented(scorer, it->key, query);
        if (!result.contains(text)) {
            result.append(text);
        }
    }
    return result;
}

QStringList GlideService::decode(const std::shared_ptr<const LanguageData> &data, const UserModel &user, const GlideQuery &query)
{
    ensureIndex();
    if (!m_index || !data || query.points.size() < 2) {
        return {};
    }
    return search(*data, user, *m_index, m_geometry, query);
}

void GlideService::decodeAsync(const std::shared_ptr<const LanguageData> &data, const UserModel &user, const GlideQuery &query)
{
    const int request = ++m_request;
    auto *watcher = new QFutureWatcher<QStringList>(this);
    connect(watcher, &QFutureWatcher<QStringList>::finished, this, [this, watcher, request] {
        watcher->deleteLater();
        if (request == m_request) {
            Q_EMIT decoded(watcher->result());
        }
    });
    ensureIndex();
    const std::shared_ptr<GlideIndex> index = m_index;
    watcher->setFuture(QtConcurrent::run([data, user, index, geometry = m_geometry, query] {
        if (!index || !data || query.points.size() < 2) {
            return QStringList();
        }
        return search(*data, user, *index, geometry, query);
    }));
}
