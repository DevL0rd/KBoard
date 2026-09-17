#pragma once

#include "usermodel.h"

#include <QObject>
#include <QTimer>

class UserStore : public QObject
{
    Q_OBJECT

public:
    explicit UserStore(QObject *parent = nullptr);
    ~UserStore() override;

    UserModel &model();
    const UserModel &model() const;

    QString open(const QString &language, const QByteArray &data);
    void close();
    void markChanged();
    void flush();

    bool exportWords(const QString &filePath) const;
    int importWords(const QString &filePath);

    static QString pathFor(const QString &language);

private:
    void save();

    UserModel m_model;
    QString m_language;
    QTimer m_saveTimer;
    bool m_dirty = false;
};
