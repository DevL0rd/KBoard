#include "gifstore.h"
#include "gifdiscovery.h"
#include "giffeed.h"
#include "gifinserter.h"
#include "giflibrary.h"
#include "klipyclient.h"

#include "kboardpaths.h"
#include "kboardsettings.h"

#include <QLocale>

namespace
{

Klipy::ContentFilter filterFromSettings()
{
    return static_cast<Klipy::ContentFilter>(std::clamp(KBoardSettings::gifContentFilter(), 0, 3));
}

qint64 cacheLimitFromSettings()
{
    return qint64(KBoardSettings::gifCacheSizeMb()) * 1024 * 1024;
}

}

GifStore::GifStore(const Options &options, QObject *parent)
    : QObject(parent)
    , m_client(new KlipyClient({options.apiKey, options.userDataDirectory, options.localeName, Klipy::ContentFilter::Medium}, this))
    , m_trending(new GifFeed(m_client, this))
    , m_results(new GifFeed(m_client, this))
    , m_library(new GifLibrary(options.userDataDirectory, this))
    , m_inserter(new GifInserter(m_client, options.cacheDirectory, this))
    , m_discovery(new GifDiscovery(m_client, this))
{
    connect(m_discovery, &GifDiscovery::categoriesChanged, this, &GifStore::categoriesChanged);
    connect(m_discovery, &GifDiscovery::suggestionsChanged, this, &GifStore::suggestionsChanged);
    connect(m_discovery, &GifDiscovery::failed, this, &GifStore::setError);
    m_library->attach(m_trending->model());
    m_library->attach(m_results->model());
    for (GifFeed *feed : {m_trending, m_results}) {
        connect(feed, &GifFeed::busyChanged, this, &GifStore::updateLoading);
        connect(feed, &GifFeed::loaded, this, &GifStore::clearError);
        connect(feed, &GifFeed::failed, this, &GifStore::setError);
    }
    connect(m_library, &GifLibrary::saveFailed, this, &GifStore::setError);
    connect(m_inserter, &GifInserter::insertingChanged, this, &GifStore::insertingChanged);
    connect(m_inserter, &GifInserter::inserted, this, &GifStore::onInserted);
    connect(m_inserter, &GifInserter::failed, this, [this](const QString &id, const QString &error) {
        setError(error);
        Q_EMIT insertFailed(id, error);
    });
    m_debounce.setSingleShot(true);
    m_debounce.setInterval(debounceInterval());
    connect(&m_debounce, &QTimer::timeout, this, &GifStore::search);
    m_error = unconfiguredError();
}

GifStore::~GifStore() = default;

GifStore *GifStore::instance()
{
    static GifStore *store = [] {
        const Options options {QStringLiteral(KBOARD_KLIPY_API_KEY).trimmed(), KBoardPaths::userDataFile(QStringLiteral("gif")),
            KBoardPaths::cacheDirectory() + QStringLiteral("/gif"), QLocale::system().name()};
        auto *created = new GifStore(options);
        created->followSettings();
        return created;
    }();
    return store;
}

GifStore *GifStore::create(QQmlEngine *, QJSEngine *)
{
    QJSEngine::setObjectOwnership(instance(), QJSEngine::CppOwnership);
    return instance();
}

void GifStore::followSettings()
{
    setContentFilter(filterFromSettings());
    m_inserter->setCacheLimit(cacheLimitFromSettings());
    connect(KBoardSettings::self(), &KBoardSettings::gifContentFilterChanged, this, [this] { setContentFilter(filterFromSettings()); });
    connect(KBoardSettings::self(), &KBoardSettings::gifCacheSizeMbChanged, this,
        [this] { m_inserter->setCacheLimit(cacheLimitFromSettings()); });
}

bool GifStore::isConfigured() const
{
    return m_client->isOffline() || m_client->isConfigured();
}

bool GifStore::isFixtureMode() const
{
    return m_client->isOffline();
}

QString GifStore::errorString() const
{
    return m_error;
}

bool GifStore::isLoading() const
{
    return m_loading;
}

bool GifStore::isLoadingMore() const
{
    return m_loadingMore;
}

bool GifStore::isInserting() const
{
    return !m_inserter->insertingId().isEmpty();
}

QString GifStore::insertingId() const
{
    return m_inserter->insertingId();
}

GifStore::MediaType GifStore::mediaType() const
{
    return m_mediaType;
}

void GifStore::setMediaType(MediaType type)
{
    if (m_mediaType != type) {
        m_mediaType = type;
        Q_EMIT mediaTypeChanged();
        refresh();
    }
}

QString GifStore::query() const
{
    return m_query;
}

void GifStore::setQuery(const QString &query)
{
    if (m_query == query) {
        return;
    }
    const bool wasSearching = isSearching();
    m_query = query;
    Q_EMIT queryChanged();
    m_debounce.stop();
    m_discovery->cancelSuggestions();
    if (isSearching() && m_client->isConfigured() && m_query.simplified() != m_searchedQuery) {
        m_results->cancel();
        m_debounce.start();
    } else if (!isSearching()) {
        m_searchedQuery.clear();
        m_results->clear();
        m_discovery->setSuggestions({});
    }
    if (wasSearching != isSearching() || isSearching()) {
        updateLoading();
    }
}

bool GifStore::isSearching() const
{
    return !m_query.trimmed().isEmpty();
}

GifModel *GifStore::trending() const
{
    return m_trending->model();
}

GifModel *GifStore::results() const
{
    return m_results->model();
}

GifModel *GifStore::favorites() const
{
    return m_library->favorites();
}

GifModel *GifStore::recents() const
{
    return m_library->recents();
}

QVariantList GifStore::categories() const
{
    return m_discovery->categories();
}

QStringList GifStore::suggestions() const
{
    return m_discovery->suggestions();
}

QString GifStore::searchPlaceholder() const
{
    return tr("Search KLIPY");
}

QString GifStore::attribution() const
{
    return tr("Powered by KLIPY");
}

int GifStore::debounceInterval()
{
    return 250;
}

KlipyClient *GifStore::client() const
{
    return m_client;
}

void GifStore::setContentFilter(Klipy::ContentFilter filter)
{
    if (m_client->contentFilter() != filter) {
        m_client->setContentFilter(filter);
        if (m_loaded) {
            refresh();
        }
    }
}

const Klipy::Item *GifStore::findItem(const QString &id) const
{
    for (const GifModel *model : {results(), trending(), recents(), favorites()}) {
        if (const Klipy::Item *item = model->find(id)) {
            return item;
        }
    }
    return nullptr;
}

void GifStore::load()
{
    if (!m_loaded) {
        refresh();
    }
}

void GifStore::refresh()
{
    if (!m_client->isConfigured() || m_client->isOffline()) {
        setError(unconfiguredError());
        return;
    }
    m_loaded = true;
    const Klipy::MediaType type = apiMediaType();
    m_trending->model()->clear();
    m_trending->reload([this, type](int page) { return Klipy::trendingUrl(m_client->context(), type, page); });
    m_discovery->fetchCategories(type);
    if (isSearching()) {
        m_results->model()->clear();
        search();
    }
}

void GifStore::retry()
{
    clearError();
    refresh();
}

void GifStore::searchNow(const QString &query)
{
    setQuery(query);
    if (m_debounce.isActive()) {
        m_debounce.stop();
        search();
    }
}

void GifStore::loadMore()
{
    activeFeed()->loadMore();
}

void GifStore::insert(const QString &id)
{
    const Klipy::Item *item = findItem(id);
    if (!item) {
        setError(tr("That GIF is no longer available"));
        return;
    }
    const bool link = KBoardSettings::gifInsertMode() == KBoardSettings::EnumGifInsertMode::Link;
    m_inserter->insert(*item, link ? GifInserter::Mode::Link : GifInserter::Mode::Image);
}

void GifStore::onInserted(const Klipy::Item &item)
{
    const bool fromSearch = isSearching() && results()->find(item.id);
    m_library->addRecent(item);
    m_client->registerShare(item, fromSearch ? m_searchedQuery : QString());
    Q_EMIT inserted(item.id);
}

void GifStore::toggleFavorite(const QString &id)
{
    if (const Klipy::Item *item = findItem(id)) {
        m_library->toggleFavorite(Klipy::Item(*item));
    }
}

bool GifStore::isFavorite(const QString &id) const
{
    return m_library->isFavorite(id);
}

void GifStore::clearRecents()
{
    m_library->clearRecents();
}

void GifStore::clearError()
{
    setError(unconfiguredError());
}

void GifStore::setError(const QString &error)
{
    const QString effective = error.isEmpty() ? unconfiguredError() : error;
    if (m_error != effective) {
        m_error = effective;
        Q_EMIT errorStringChanged();
    }
}

QString GifStore::unconfiguredError() const
{
    return isConfigured() ? QString() : tr("GIFs need a KLIPY API key at build time");
}

void GifStore::search()
{
    const QString text = m_query.simplified();
    if (text.isEmpty() || !m_client->isConfigured()) {
        return;
    }
    m_searchedQuery = text;
    const Klipy::MediaType type = apiMediaType();
    m_results->reload([this, type, text](int page) { return Klipy::searchUrl(m_client->context(), type, text, page); });
    m_discovery->fetchSuggestions(text);
    updateLoading();
}

GifFeed *GifStore::activeFeed() const
{
    return isSearching() ? m_results : m_trending;
}

void GifStore::updateLoading()
{
    const GifFeed *feed = activeFeed();
    const bool pending = feed->isBusy() || (isSearching() && m_debounce.isActive());
    const bool loading = pending && feed->isEmpty();
    const bool loadingMore = feed->isBusy() && !loading;
    if (loading != m_loading || loadingMore != m_loadingMore) {
        m_loading = loading;
        m_loadingMore = loadingMore;
        Q_EMIT loadingChanged();
    }
}

Klipy::MediaType GifStore::apiMediaType() const
{
    return m_mediaType == Stickers ? Klipy::MediaType::Stickers : Klipy::MediaType::Gifs;
}
