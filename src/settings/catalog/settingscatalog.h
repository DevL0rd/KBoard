#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QVariantList>
#include <QVariantMap>

class SettingsCatalog : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QVariantList pages READ pages CONSTANT)
    Q_PROPERTY(QVariantList entries READ entries CONSTANT)
    Q_PROPERTY(QString errorString READ errorString CONSTANT)

public:
    explicit SettingsCatalog(QObject *parent = nullptr);
    explicit SettingsCatalog(const QString &path, QObject *parent = nullptr);

    QVariantList pages() const;
    QVariantList entries() const;
    QString errorString() const;

    Q_INVOKABLE QVariantMap page(const QString &id) const;
    Q_INVOKABLE bool hasPage(const QString &id) const;
    Q_INVOKABLE QVariantList search(const QString &query, int limit = 40) const;
    Q_INVOKABLE QVariantList entriesForPage(const QString &id) const;

private:
    void load(const QString &path);

    QVariantList m_pages;
    QVariantList m_entries;
    QString m_errorString;
};
