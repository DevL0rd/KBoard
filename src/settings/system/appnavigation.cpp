#include "appnavigation.h"

AppNavigation::AppNavigation(QObject *parent)
    : QObject(parent)
{ }

AppNavigation *AppNavigation::instance()
{
    static AppNavigation *navigation = new AppNavigation;
    return navigation;
}

AppNavigation *AppNavigation::create(QQmlEngine *, QJSEngine *)
{
    QJSEngine::setObjectOwnership(instance(), QJSEngine::CppOwnership);
    return instance();
}

QString AppNavigation::page() const
{
    return m_page;
}

void AppNavigation::setPage(const QString &page)
{
    if (page == m_page) {
        return;
    }
    m_page = page;
    Q_EMIT pageChanged();
}

QString AppNavigation::searchText() const
{
    return m_searchText;
}

void AppNavigation::setSearchText(const QString &text)
{
    if (text == m_searchText) {
        return;
    }
    m_searchText = text;
    Q_EMIT searchTextChanged();
}

QString AppNavigation::revealLabel() const
{
    return m_revealLabel;
}

int AppNavigation::revealSerial() const
{
    return m_revealSerial;
}

void AppNavigation::open(const QString &page, const QString &label)
{
    setPage(page);
    if (!label.isEmpty()) {
        m_revealLabel = label;
        ++m_revealSerial;
        Q_EMIT revealRequested();
    }
}
