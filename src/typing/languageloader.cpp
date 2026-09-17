#include "languageloader.h"

#include <QtConcurrentRun>

LanguageLoader::LanguageLoader(QObject *parent)
    : QObject(parent)
{
    connect(&m_watcher, &QFutureWatcher<LanguageLoad>::finished, this, [this] {
        const LanguageLoad result = m_watcher.result();
        m_pendingLocale.clear();
        Q_EMIT loadingChanged();
        Q_EMIT loaded(result);
    });
}

void LanguageLoader::load(const LanguageSource &source)
{
    const bool wasLoading = isLoading();
    m_pendingLocale = source.locale;
    m_watcher.setFuture(QtConcurrent::run([source] { return loadLanguage(source); }));
    if (!wasLoading) {
        Q_EMIT loadingChanged();
    }
}

bool LanguageLoader::isLoading() const
{
    return !m_pendingLocale.isEmpty();
}

QString LanguageLoader::pendingLocale() const
{
    return m_pendingLocale;
}
