#pragma once

#include <QObject>

class QDBusServiceWatcher;

class Haptics : public QObject
{
    Q_OBJECT

public:
    explicit Haptics(QObject *parent = nullptr);

    bool available() const;
    void trigger(const QString &event);

Q_SIGNALS:
    void availableChanged();

private:
    void setAvailable(bool available);

    QDBusServiceWatcher *m_watcher;
    bool m_available = false;
};
