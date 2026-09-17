#pragma once

#include "clipboardentry.h"
#include "clipboardexpiry.h"
#include "clipboardguard.h"
#include "clipboardhistoryfile.h"
#include "clipboardimagestore.h"
#include "clipboardlatest.h"
#include "clipboardpaster.h"
#include "clipboardroles.h"

#include "kboardsettings.h"

#include <QAbstractListModel>
#include <QQmlEngine>
#include <QTimer>

class QMimeData;
class KlipperBridge;
class SystemClipboard;

class ClipboardHistory : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QVariantMap latest READ latest NOTIFY latestChanged)
    Q_PROPERTY(bool latestFresh READ latestFresh NOTIFY latestFreshChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(int pinnedCount READ pinnedCount NOTIFY countChanged)
    Q_PROPERTY(bool paused READ isPaused WRITE setPaused NOTIFY pausedChanged)
    Q_PROPERTY(bool enabled READ isEnabled NOTIFY enabledChanged)
    Q_PROPERTY(bool watching READ isWatching CONSTANT)
    Q_PROPERTY(bool klipperConnected READ isKlipperConnected NOTIFY klipperConnectedChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorStringChanged)

public:
    struct Options
    {
        QString dataDirectory;
        QString cacheDirectory;
        bool watchSystem = false;
        ClipboardGuard::SensitiveProbe sensitiveProbe;
        ClipboardExpiry::Clock clock;
    };

    explicit ClipboardHistory(const Options &options, QObject *parent = nullptr);
    ~ClipboardHistory() override;

    static ClipboardHistory *instance();
    static void setInstance(ClipboardHistory *history);
    static ClipboardHistory *create(QQmlEngine *engine, QJSEngine *);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override { return ClipboardRoles::names(); }

    int count() const { return int(m_entries.size()); }
    int pinnedCount() const;
    QVariantMap latest() const { return m_latest.value(); }
    bool latestFresh() const { return m_latest.isFresh(); }
    static bool isPaused() { return KBoardSettings::clipboardPaused(); }
    void setPaused(bool paused);
    static bool isEnabled() { return KBoardSettings::clipboardEnabled(); }
    bool isWatching() const { return m_watching; }
    bool isKlipperConnected() const;
    QString errorString() const { return m_errorString; }

    const ClipboardEntry &entryAt(int row) const { return m_entries.at(row); }
    const ClipboardExpiry &expiry() const { return m_expiry; }
    const ClipboardImageStore &images() const { return m_images; }
    QString historyFile() const { return m_file.path(); }

    bool addMimeData(const QMimeData *mime);
    bool addText(const QString &text);
    bool addImage(const QImage &image, const QString &mimeType = QStringLiteral("image/png"));
    void mergeKlipperHistory(const QStringList &history, bool onlyNewest);
    void setSensitiveProbe(const ClipboardGuard::SensitiveProbe &probe) { m_guard.setSensitiveProbe(probe); }
    void expireDue();
    void saveNow();
    void waitForPendingWrites() { m_images.waitForWrites(); }

    Q_INVOKABLE bool paste(int row);
    Q_INVOKABLE bool pasteLatest();
    Q_INVOKABLE bool copyToClipboard(int row);
    Q_INVOKABLE void pin(int row, bool pinned);
    Q_INVOKABLE void remove(int row);
    Q_INVOKABLE void clear(bool includePinned = false);
    Q_INVOKABLE int rowForId(const QString &id) const;
    Q_INVOKABLE QVariantMap get(int row) const;
    Q_INVOKABLE void dismissLatest() { m_latest.dismiss(); }

Q_SIGNALS:
    void latestChanged();
    void latestFreshChanged();
    void countChanged();
    void pausedChanged();
    void enabledChanged();
    void klipperConnectedChanged();
    void errorStringChanged();
    void pasted(int row);

private:
    void connectSettings();
    void startWatching();
    void load();
    bool isValidRow(int row) const { return row >= 0 && row < m_entries.size(); }
    int findText(const QString &text) const;
    void insertEntry(ClipboardEntry entry, bool announce);
    bool bumpExisting(int row);
    void removeEntries(QList<int> rows);
    void enforceLimit();
    void entriesChanged();
    void clearExpiringFlags();
    static bool shouldPersist(const ClipboardEntry &entry);
    void setError(const QString &error);
    bool reportPaster(bool ok);

    Options m_options;
    QList<ClipboardEntry> m_entries;
    ClipboardImageStore m_images;
    ClipboardHistoryFile m_file;
    ClipboardExpiry m_expiry;
    ClipboardLatest m_latest;
    ClipboardGuard m_guard;
    ClipboardPaster m_paster;
    QTimer m_saveTimer;
    KlipperBridge *m_klipper = nullptr;
    bool m_watching = false;
    QString m_errorString;
};
