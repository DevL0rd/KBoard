#pragma once

#include "transcripttext.h"

#include <QList>
#include <QString>

class InputContext;

class TranscriptWriter
{
public:
    explicit TranscriptWriter(InputContext *input);

    void reset();
    void preview(const QString &partial);
    QString write(const QString &transcript, bool commands);
    QString committedText() const;

private:
    QString textBefore() const;
    void insert(const QString &text, QString &before, QString &written);
    void newLine(QString &before, QString &written);
    void deletePrevious(QString &before);

    InputContext *m_input;
    QList<int> m_phraseLengths;
    QString m_committedText;
};
