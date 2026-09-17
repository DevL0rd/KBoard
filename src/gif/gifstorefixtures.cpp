#include "gifdiscovery.h"
#include "gifstore.h"
#include "klipyclient.h"

#include <QDir>
#include <QFile>

namespace
{

QByteArray readFixture(const QDir &dir, const QString &name)
{
    QFile file(dir.filePath(name));
    return file.open(QIODevice::ReadOnly) ? file.readAll() : QByteArray();
}

}

bool GifStore::loadFixtures(const QString &directory)
{
    const QDir dir(directory);
    Klipy::Page page;
    QString error;
    if (!Klipy::parsePage(readFixture(dir, QStringLiteral("trending.json")), &page, &error)) {
        setError(tr("Could not load fixtures from %1: %2").arg(directory, error));
        return false;
    }
    m_client->setOffline(true);
    m_loaded = true;
    Q_EMIT configuredChanged();
    trending()->setItems(page.items);
    if (Klipy::parsePage(readFixture(dir, QStringLiteral("search.json")), &page, &error)) {
        results()->setItems(page.items);
    }
    QList<Klipy::Category> categories;
    if (Klipy::parseCategories(readFixture(dir, QStringLiteral("categories.json")), &categories, &error)) {
        m_discovery->setCategories(categories);
    }
    QStringList suggestions;
    if (Klipy::parseStringList(readFixture(dir, QStringLiteral("autocomplete.json")), &suggestions, &error)) {
        m_discovery->setSuggestions(suggestions);
    }
    setError({});
    return true;
}
