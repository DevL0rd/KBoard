#pragma once

#include "gifmodel.h"
#include "klipyapi.h"

#include <QObject>
#include <QQmlEngine>
#include <QTimer>
#include <QVariantList>

class GifDiscovery;
class GifFeed;
class GifInserter;
class GifLibrary;
class KlipyClient;

class GifStore : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(bool configured READ isConfigured NOTIFY configuredChanged)
    Q_PROPERTY(bool fixtureMode READ isFixtureMode NOTIFY configuredChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorStringChanged)
    Q_PROPERTY(bool loading READ isLoading NOTIFY loadingChanged)
    Q_PROPERTY(bool loadingMore READ isLoadingMore NOTIFY loadingChanged)
    Q_PROPERTY(bool inserting READ isInserting NOTIFY insertingChanged)
    Q_PROPERTY(QString insertingId READ insertingId NOTIFY insertingChanged)
    Q_PROPERTY(MediaType mediaType READ mediaType WRITE setMediaType NOTIFY mediaTypeChanged)
    Q_PROPERTY(QString query READ query WRITE setQuery NOTIFY queryChanged)
    Q_PROPERTY(bool searching READ isSearching NOTIFY queryChanged)
    Q_PROPERTY(GifModel *trending READ trending CONSTANT)
    Q_PROPERTY(GifModel *results READ results CONSTANT)
    Q_PROPERTY(GifModel *favorites READ favorites CONSTANT)
    Q_PROPERTY(GifModel *recents READ recents CONSTANT)
    Q_PROPERTY(QVariantList categories READ categories NOTIFY categoriesChanged)
    Q_PROPERTY(QStringList suggestions READ suggestions NOTIFY suggestionsChanged)
    Q_PROPERTY(QString searchPlaceholder READ searchPlaceholder CONSTANT)
    Q_PROPERTY(QString attribution READ attribution CONSTANT)

public:
    enum MediaType
    {
        Gifs,
        Stickers,
    };
    Q_ENUM(MediaType)

    struct Options
    {
        QString apiKey;
        QString userDataDirectory;
        QString cacheDirectory;
        QString localeName;
    };

    explicit GifStore(const Options &options, QObject *parent = nullptr);
    ~GifStore() override;

    static GifStore *instance();
    static GifStore *create(QQmlEngine *, QJSEngine *);

    bool isConfigured() const;
    bool isFixtureMode() const;
    QString errorString() const;
    bool isLoading() const;
    bool isLoadingMore() const;
    bool isInserting() const;
    QString insertingId() const;
    MediaType mediaType() const;
    void setMediaType(MediaType type);
    QString query() const;
    void setQuery(const QString &query);
    bool isSearching() const;
    GifModel *trending() const;
    GifModel *results() const;
    GifModel *favorites() const;
    GifModel *recents() const;
    QVariantList categories() const;
    QStringList suggestions() const;
    QString searchPlaceholder() const;
    QString attribution() const;

    static int debounceInterval();
    KlipyClient *client() const;
    void setContentFilter(Klipy::ContentFilter filter);
    const Klipy::Item *findItem(const QString &id) const;

    Q_INVOKABLE void load();
    Q_INVOKABLE void refresh();
    Q_INVOKABLE void retry();
    Q_INVOKABLE void searchNow(const QString &query);
    Q_INVOKABLE void loadMore();
    Q_INVOKABLE void insert(const QString &id);
    Q_INVOKABLE void toggleFavorite(const QString &id);
    Q_INVOKABLE bool isFavorite(const QString &id) const;
    Q_INVOKABLE void clearRecents();
    Q_INVOKABLE void clearError();
    Q_INVOKABLE bool loadFixtures(const QString &directory);

Q_SIGNALS:
    void configuredChanged();
    void errorStringChanged();
    void loadingChanged();
    void insertingChanged();
    void mediaTypeChanged();
    void queryChanged();
    void categoriesChanged();
    void suggestionsChanged();
    void inserted(const QString &id);
    void insertFailed(const QString &id, const QString &error);

private:
    void followSettings();
    void setError(const QString &error);
    void search();
    void updateLoading();
    void onInserted(const Klipy::Item &item);
    GifFeed *activeFeed() const;
    Klipy::MediaType apiMediaType() const;
    QString unconfiguredError() const;

    KlipyClient *m_client;
    GifFeed *m_trending;
    GifFeed *m_results;
    GifLibrary *m_library;
    GifInserter *m_inserter;
    GifDiscovery *m_discovery;
    QTimer m_debounce;
    QString m_error;
    MediaType m_mediaType = Gifs;
    QString m_query;
    QString m_searchedQuery;
    bool m_loaded = false;
    bool m_loading = false;
    bool m_loadingMore = false;
};
