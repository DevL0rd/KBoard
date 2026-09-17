#pragma once

#include <QObject>

class ActiveAppWatcher : public QObject
{
    Q_OBJECT

public:
    explicit ActiveAppWatcher(QObject *parent = nullptr);

    void start();

private:
    void runScript(int id);

    QString m_pluginName;
};
