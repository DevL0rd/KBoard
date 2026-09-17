#pragma once

#include "edithistory.h"

#include <KConfigWatcher>

#include <QElapsedTimer>
#include <QObject>
#include <QQmlEngine>
#include <QStringList>
#include <QTimer>
#include <QUrl>
#include <QVariant>

class KCoreConfigSkeleton;
class KConfigSkeletonItem;

class SettingsStore : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(int revision READ revision NOTIFY revisionChanged)
    Q_PROPERTY(bool canUndo READ canUndo NOTIFY historyChanged)
    Q_PROPERTY(int undoCount READ undoCount NOTIFY historyChanged)
    Q_PROPERTY(bool representsDefaults READ representsDefaults NOTIFY revisionChanged)
    Q_PROPERTY(QString configPath READ configPath CONSTANT)
    Q_PROPERTY(QStringList entryNames READ entryNames CONSTANT)

public:
    static SettingsStore *create(QQmlEngine *, QJSEngine *);
    static SettingsStore *instance();

    int revision() const;
    bool canUndo() const;
    int undoCount() const;
    bool representsDefaults() const;
    QString configPath() const;
    QStringList entryNames() const;

    Q_INVOKABLE bool has(const QString &name) const;
    Q_INVOKABLE QVariant value(const QString &name) const;
    Q_INVOKABLE bool setValue(const QString &name, const QVariant &value);
    Q_INVOKABLE bool setValues(const QVariantMap &values);
    Q_INVOKABLE bool isDefault(const QString &name) const;
    Q_INVOKABLE QVariant defaultValue(const QString &name) const;
    Q_INVOKABLE bool resetToDefault(const QString &name);
    Q_INVOKABLE QString groupOf(const QString &name) const;
    Q_INVOKABLE void defaults();
    Q_INVOKABLE bool undo();
    Q_INVOKABLE bool exportTo(const QUrl &url);
    Q_INVOKABLE bool importFrom(const QUrl &url);

    void flush();
    bool exportToFile(const QString &path, QString *error) const;
    bool importFromFile(const QString &path, QString *error);

Q_SIGNALS:
    void revisionChanged();
    void historyChanged();
    void applied(const QString &message);
    void failed(const QString &message);

private Q_SLOTS:
    void bump();

private:
    explicit SettingsStore(QObject *parent = nullptr);
    KConfigSkeletonItem *item(const QString &name) const;
    QVariant normalized(KConfigSkeletonItem *entry, const QVariant &value) const;
    void commit(const QString &message);
    void reloadFromDisk();

    KCoreConfigSkeleton *m_skeleton;
    EditHistory m_history;
    QTimer m_saveTimer;
    KConfigWatcher::Ptr m_watcher;
    QString m_pendingMessage;
    QElapsedTimer m_clock;
    int m_revision = 0;
};
