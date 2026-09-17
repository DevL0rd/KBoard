#pragma once

#include "klipyapi.h"

#include <QObject>
#include <QPointer>
#include <QVariantList>

class KlipyClient;
class QNetworkReply;

class GifDiscovery : public QObject
{
    Q_OBJECT

public:
    explicit GifDiscovery(KlipyClient *client, QObject *parent = nullptr);

    QVariantList categories() const;
    QStringList suggestions() const;
    void fetchCategories(Klipy::MediaType type);
    void fetchSuggestions(const QString &query);
    void cancelSuggestions();
    void setCategories(const QList<Klipy::Category> &categories);
    void setSuggestions(const QStringList &suggestions);

Q_SIGNALS:
    void categoriesChanged();
    void suggestionsChanged();
    void failed(const QString &error);

private:
    KlipyClient *m_client;
    QVariantList m_categories;
    QStringList m_suggestions;
    QPointer<QNetworkReply> m_categoriesReply;
    QPointer<QNetworkReply> m_suggestionsReply;
};
