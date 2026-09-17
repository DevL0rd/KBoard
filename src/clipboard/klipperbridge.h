#pragma once

#include <QObject>
#include <QStringList>

class QDBusServiceWatcher;

class KlipperBridge : public QObject
{
    Q_OBJECT

public:
    explicit KlipperBridge(QObject *parent = nullptr);

    bool isConnected() const;
    void fetch(bool onlyNewest);

Q_SIGNALS:
    void connectedChanged();
    void historyReceived(const QStringList &history, bool onlyNewest);

private:
    Q_SLOT void onHistoryUpdated();
    void setConnected(bool connected);

    QDBusServiceWatcher *m_watcher = nullptr;
    bool m_connected = false;
};
