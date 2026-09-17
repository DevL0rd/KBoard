#include "giflibrary.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSaveFile>

namespace
{

const int recentsLimit = 48;
const int favoritesLimit = 500;

}

GifLibrary::GifLibrary(const QString &directory, QObject *parent)
    : QObject(parent)
    , m_directory(directory)
    , m_favorites(new GifModel(this))
    , m_recents(new GifModel(this))
{
    restore(m_favorites, favoritesFile());
    restore(m_recents, recentsFile());
    for (const Klipy::Item &item : m_favorites->items()) {
        m_favoriteIds.insert(item.id);
    }
    attach(m_favorites);
    attach(m_recents);
}

GifModel *GifLibrary::favorites() const
{
    return m_favorites;
}

GifModel *GifLibrary::recents() const
{
    return m_recents;
}

void GifLibrary::attach(GifModel *model)
{
    model->setFavoriteIds(&m_favoriteIds);
    connect(this, &GifLibrary::favoriteChanged, model, &GifModel::favoriteChanged);
}

bool GifLibrary::isFavorite(const QString &id) const
{
    return m_favoriteIds.contains(id);
}

void GifLibrary::toggleFavorite(const Klipy::Item &item)
{
    if (isFavorite(item.id)) {
        removeFavorite(item.id);
        return;
    }
    m_favoriteIds.insert(item.id);
    m_favorites->prepend(item, favoritesLimit);
    persist(m_favorites, favoritesFile());
    Q_EMIT favoriteChanged(item.id);
}

void GifLibrary::removeFavorite(const QString &id)
{
    m_favoriteIds.remove(id);
    m_favorites->remove(id);
    persist(m_favorites, favoritesFile());
    Q_EMIT favoriteChanged(id);
}

void GifLibrary::addRecent(const Klipy::Item &item)
{
    m_recents->prepend(item, recentsLimit);
    persist(m_recents, recentsFile());
}

void GifLibrary::clearRecents()
{
    m_recents->clear();
    persist(m_recents, recentsFile());
}

void GifLibrary::persist(const GifModel *model, const QString &path)
{
    QJsonArray array;
    for (const Klipy::Item &item : model->items()) {
        array.append(item.toJson());
    }
    QDir().mkpath(m_directory);
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly) || file.write(QJsonDocument(array).toJson(QJsonDocument::Compact)) < 0 || !file.commit()) {
        Q_EMIT saveFailed(tr("Could not save %1: %2").arg(path, file.errorString()));
    }
}

void GifLibrary::restore(GifModel *model, const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }
    QList<Klipy::Item> items;
    const QJsonArray array = QJsonDocument::fromJson(file.readAll()).array();
    for (const auto &value : array) {
        Klipy::Item item = Klipy::Item::fromJson(value.toObject());
        if (item.isValid()) {
            items.append(std::move(item));
        }
    }
    model->setItems(items);
}

QString GifLibrary::favoritesFile() const
{
    return m_directory + QStringLiteral("/favorites.json");
}

QString GifLibrary::recentsFile() const
{
    return m_directory + QStringLiteral("/recents.json");
}
