#pragma once

#include <QObject>
#include <QQmlEngine>

class AppNavigation : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QString page READ page WRITE setPage NOTIFY pageChanged)
    Q_PROPERTY(QString revealLabel READ revealLabel NOTIFY revealRequested)
    Q_PROPERTY(int revealSerial READ revealSerial NOTIFY revealRequested)
    Q_PROPERTY(QString searchText READ searchText WRITE setSearchText NOTIFY searchTextChanged)

public:
    static AppNavigation *instance();
    static AppNavigation *create(QQmlEngine *, QJSEngine *);

    QString page() const;
    void setPage(const QString &page);
    QString revealLabel() const;
    int revealSerial() const;

    QString searchText() const;
    void setSearchText(const QString &text);

    Q_INVOKABLE void open(const QString &page, const QString &label = QString());

Q_SIGNALS:
    void pageChanged();
    void searchTextChanged();
    void revealRequested();
    void raiseRequested();

private:
    explicit AppNavigation(QObject *parent = nullptr);
    QString m_page = QStringLiteral("look");
    QString m_revealLabel;
    QString m_searchText;
    int m_revealSerial = 0;
};
