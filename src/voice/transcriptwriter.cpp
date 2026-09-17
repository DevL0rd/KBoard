#include "transcriptwriter.h"

#include "inputcontext.h"

TranscriptWriter::TranscriptWriter(InputContext *input)
    : m_input(input)
{ }

void TranscriptWriter::reset()
{
    m_phraseLengths.clear();
    m_committedText.clear();
}

void TranscriptWriter::preview(const QString &partial)
{
    m_input->setPreedit(TranscriptText::fitToContext(partial, textBefore()));
}

QString TranscriptWriter::textBefore() const
{
    return m_input->hasSurroundingText() ? m_input->textBeforeCursor() : m_committedText;
}

QString TranscriptWriter::write(const QString &transcript, bool commands)
{
    const QList<TranscriptText::Action> actions = commands ? TranscriptText::applyCommands(transcript) : TranscriptText::plain(transcript);
    if (actions.isEmpty() || actions.first().kind != TranscriptText::Action::Insert) {
        m_input->clearPreedit();
    }
    QString before = textBefore();
    QString written;
    for (const TranscriptText::Action &action : actions) {
        switch (action.kind) {
        case TranscriptText::Action::Insert:
            insert(action.text, before, written);
            break;
        case TranscriptText::Action::NewLine:
            newLine(before, written);
            break;
        case TranscriptText::Action::DeletePrevious:
            deletePrevious(before);
            break;
        }
    }
    return written;
}

QString TranscriptWriter::committedText() const
{
    return m_committedText;
}

void TranscriptWriter::insert(const QString &text, QString &before, QString &written)
{
    const QString fitted = TranscriptText::fitToContext(text, before);
    if (fitted.isEmpty()) {
        return;
    }
    m_input->commit(fitted);
    before += fitted;
    written += fitted;
    m_committedText += fitted;
    m_phraseLengths.append(int(fitted.size()));
}

void TranscriptWriter::newLine(QString &before, QString &written)
{
    m_input->clearPreedit();
    m_input->enter();
    before += QLatin1Char('\n');
    written += QLatin1Char('\n');
    m_committedText += QLatin1Char('\n');
    m_phraseLengths.append(1);
}

void TranscriptWriter::deletePrevious(QString &before)
{
    if (m_phraseLengths.isEmpty()) {
        return;
    }
    m_input->clearPreedit();
    const int length = m_phraseLengths.takeLast();
    if (m_input->hasSurroundingText()) {
        m_input->deleteSurrounding(length, 0);
    } else {
        for (int i = 0; i < length; ++i) {
            m_input->backspace();
        }
    }
    before.chop(length);
    m_committedText.chop(qMin(length, int(m_committedText.size())));
}
