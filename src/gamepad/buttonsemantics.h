#pragma once

#include "chordmatcher.h"

#include <QObject>
#include <QSet>
#include <QStringList>

class ButtonSemantics : public QObject
{
    Q_OBJECT

public:
    explicit ButtonSemantics(QObject *parent = nullptr);

    bool setChord(const QString &chord, const QSet<QString> &held);
    QStringList chord() const;
    QString chordError() const;

    void press(const QString &button);
    void release(const QString &button);
    void releaseAll(const QSet<QString> &held);

Q_SIGNALS:
    void pressed(const QString &button);
    void released(const QString &button);
    void tapped(const QString &button);
    void chordActivated();

private:
    ChordMatcher m_matcher;
    QSet<QString> m_chordConsumed;
    QSet<QString> m_suppressed;
};
