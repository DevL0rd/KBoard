#pragma once

#include "languagedata.h"

#include <QFutureWatcher>
#include <QObject>

class LanguageLoader : public QObject
{
    Q_OBJECT

public:
    explicit LanguageLoader(QObject *parent = nullptr);

    void load(const LanguageSource &source);
    bool isLoading() const;
    QString pendingLocale() const;

Q_SIGNALS:
    void loadingChanged();
    void loaded(const LanguageLoad &result);

private:
    QFutureWatcher<LanguageLoad> m_watcher;
    QString m_pendingLocale;
};
