#pragma once

#include <QObject>
#include <QTimer>
#include <QVariantMap>

class ClipboardLatest : public QObject
{
    Q_OBJECT

public:
    explicit ClipboardLatest(QObject *parent = nullptr);

    QString id() const;
    QVariantMap value() const;
    bool isFresh() const;

    void announce(const QString &id, const QVariantMap &value);
    void refresh(const QString &id, const QVariantMap &value);
    void forget(const QString &id);
    void dismiss();

Q_SIGNALS:
    void valueChanged();
    void freshChanged();

private:
    void setFresh(bool fresh);

    QString m_id;
    QVariantMap m_value;
    bool m_fresh = false;
    QTimer m_timer;
};
