#pragma once

#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QJsonObject>
#include <QObject>
#include <QQmlEngine>
#include <QVariantMap>

class Layouts : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QVariantList available READ available NOTIFY changed)
    Q_PROPERTY(QString errorString READ errorString NOTIFY changed)
    Q_PROPERTY(QString directory READ directory NOTIFY changed)
    Q_PROPERTY(int revision READ revision NOTIFY changed)

public:
    explicit Layouts(const QString &directory, QObject *parent = nullptr);
    static Layouts *create(QQmlEngine *, QJSEngine *);

    QVariantList available() const;
    QString errorString() const;
    QString directory() const;
    int revision() const;

    Q_INVOKABLE bool has(const QString &id) const;
    Q_INVOKABLE QVariantMap layout(const QString &id) const;
    Q_INVOKABLE QVariantMap page(const QString &layoutId, const QString &pageId, const QVariantMap &options = {}) const;
    Q_INVOKABLE QStringList pages() const;
    Q_INVOKABLE void reload();

Q_SIGNALS:
    void changed();

private:
    QString loadFile(const QFileInfo &info);
    void watchFiles(const QFileInfoList &files);
    void sortLayouts();
    QVariantList resolveRows(const QJsonObject &layout, const QJsonArray &rows, const QVariantMap &options, int columns) const;
    QVariantMap resolveRow(const QJsonObject &layout, const QJsonObject &row, const QVariantMap &options, int columns) const;
    QJsonObject namedRow(const QJsonObject &layout, const QString &name) const;
    QVariantMap resolveKey(const QJsonObject &layout, const QJsonObject &key, bool rowGlide, bool rowSpecial) const;
    bool conditionHolds(const QString &condition, const QVariantMap &options) const;

    QString m_directory;
    QString m_errorString;
    QHash<QString, QJsonObject> m_letters;
    QHash<QString, QJsonObject> m_pages;
    QJsonObject m_rows;
    QStringList m_order;
    QFileSystemWatcher m_watcher;
    int m_revision = 0;
};
