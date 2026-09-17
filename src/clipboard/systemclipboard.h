#pragma once

#include <QObject>

class QImage;
class QMimeData;

class SystemClipboard : public QObject
{
    Q_OBJECT

public:
    explicit SystemClipboard(QObject *parent = nullptr);

    bool start();
    QString errorString() const;
    bool publishText(const QString &text);
    bool publishImage(const QImage &image);

Q_SIGNALS:
    void copied(const QMimeData *mime);

private:
    bool publish(QMimeData *mime);

    QString m_errorString;
};
