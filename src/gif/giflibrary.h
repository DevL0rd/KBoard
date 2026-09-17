#pragma once

#include "gifmodel.h"

#include <QObject>
#include <QSet>

class GifLibrary : public QObject
{
    Q_OBJECT

public:
    explicit GifLibrary(const QString &directory, QObject *parent = nullptr);

    GifModel *favorites() const;
    GifModel *recents() const;
    void attach(GifModel *model);
    bool isFavorite(const QString &id) const;
    void toggleFavorite(const Klipy::Item &item);
    void removeFavorite(const QString &id);
    void addRecent(const Klipy::Item &item);
    void clearRecents();

Q_SIGNALS:
    void favoriteChanged(const QString &id);
    void saveFailed(const QString &error);

private:
    void persist(const GifModel *model, const QString &path);
    void restore(GifModel *model, const QString &path);
    QString favoritesFile() const;
    QString recentsFile() const;

    QString m_directory;
    GifModel *m_favorites;
    GifModel *m_recents;
    QSet<QString> m_favoriteIds;
};
