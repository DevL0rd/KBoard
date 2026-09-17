#pragma once

#include <QSet>
#include <QString>
#include <QStringList>

class ChordMatcher
{
public:
    struct Parsed
    {
        QStringList buttons;
        QString error;
    };

    static Parsed parse(const QString &chord);

    bool setChord(const QString &chord);
    QStringList buttons() const;
    QString errorString() const;
    bool isValid() const;

    bool press(const QString &button);
    void release(const QString &button);
    void reset();

private:
    QStringList m_buttons;
    QString m_error;
    QSet<QString> m_held;
    bool m_fired = false;
};
