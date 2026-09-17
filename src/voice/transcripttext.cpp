#include "transcripttext.h"

#include <QStringList>

namespace
{
enum class CommandKind
{
    Punctuation,
    NewLine,
    NewParagraph,
    DeleteThat,
};

struct Command
{
    QStringList words;
    CommandKind kind;
    QChar symbol;
};

const QList<Command> &commands()
{
    static const QList<Command> list {
        {{QStringLiteral("new"), QStringLiteral("paragraph")}, CommandKind::NewParagraph, QChar()},
        {{QStringLiteral("new"), QStringLiteral("line")}, CommandKind::NewLine, QChar()},
        {{QStringLiteral("newline")}, CommandKind::NewLine, QChar()},
        {{QStringLiteral("delete"), QStringLiteral("that")}, CommandKind::DeleteThat, QChar()},
        {{QStringLiteral("full"), QStringLiteral("stop")}, CommandKind::Punctuation, u'.'},
        {{QStringLiteral("period")}, CommandKind::Punctuation, u'.'},
        {{QStringLiteral("comma")}, CommandKind::Punctuation, u','},
        {{QStringLiteral("question"), QStringLiteral("mark")}, CommandKind::Punctuation, u'?'},
        {{QStringLiteral("exclamation"), QStringLiteral("mark")}, CommandKind::Punctuation, u'!'},
        {{QStringLiteral("exclamation"), QStringLiteral("point")}, CommandKind::Punctuation, u'!'},
    };
    return list;
}

bool isWordChar(QChar c)
{
    return c.isLetterOrNumber() || c == u'\'' || c == u'’';
}

QString normalized(const QString &word)
{
    qsizetype start = 0;
    qsizetype end = word.size();
    while (start < end && !isWordChar(word.at(start))) {
        ++start;
    }
    while (end > start && !isWordChar(word.at(end - 1))) {
        --end;
    }
    return word.mid(start, end - start).toLower();
}

bool isTrailingPunctuation(QChar c)
{
    return c == u'.' || c == u',' || c == u'!' || c == u'?' || c == u';' || c == u':' || c == u'…';
}

bool endsSentence(QChar c)
{
    return c == u'.' || c == u'!' || c == u'?' || c == u'…';
}

bool isPronounI(const QString &word)
{
    const QString bare = normalized(word);
    return bare == u"i" || bare.startsWith(u"i'") || bare.startsWith(u"i’");
}

QString capitalized(const QString &word)
{
    for (qsizetype i = 0; i < word.size(); ++i) {
        if (word.at(i).isLetter()) {
            QString result = word;
            result[i] = word.at(i).toUpper();
            return result;
        }
    }
    return word;
}

QString decapitalized(const QString &word)
{
    if (isPronounI(word)) {
        return word;
    }
    qsizetype first = -1;
    qsizetype letters = 0;
    bool restUpper = true;
    for (qsizetype i = 0; i < word.size(); ++i) {
        const QChar c = word.at(i);
        if (!c.isLetter()) {
            continue;
        }
        if (first < 0) {
            first = i;
        } else if (!c.isUpper()) {
            restUpper = false;
        }
        ++letters;
    }
    if (first < 0 || !word.at(first).isUpper()) {
        return word;
    }
    if (letters > 1 && restUpper) {
        return word;
    }
    QString result = word;
    result[first] = word.at(first).toLower();
    return result;
}

const Command *matchCommand(const QStringList &normalizedWords, qsizetype index)
{
    for (const Command &command : commands()) {
        if (index + command.words.size() > normalizedWords.size()) {
            continue;
        }
        bool matches = true;
        for (qsizetype k = 0; k < command.words.size(); ++k) {
            if (normalizedWords.at(index + k) != command.words.at(k)) {
                matches = false;
                break;
            }
        }
        if (matches) {
            return &command;
        }
    }
    return nullptr;
}

void chopTrailingPunctuation(QString &text)
{
    while (!text.isEmpty() && isTrailingPunctuation(text.back())) {
        text.chop(1);
    }
}

bool isClosingPunctuation(QChar c)
{
    return isTrailingPunctuation(c) || c == u')' || c == u']' || c == u'}' || c == u'%';
}

bool opensGroup(QChar c)
{
    return c == u'(' || c == u'[' || c == u'{' || c == u'"' || c == u'\u201C' || c == u'\u2018';
}

bool isSentenceStart(const QString &before)
{
    qsizetype end = before.size();
    while (end > 0 && before.at(end - 1).isSpace() && before.at(end - 1) != u'\n') {
        --end;
    }
    return end == 0 || before.at(end - 1) == u'\n' || endsSentence(before.at(end - 1));
}

using TranscriptText::Action;

class CommandApplier
{
public:
    explicit CommandApplier(const QString &transcript)
        : m_words(transcript.simplified().split(u' ', Qt::SkipEmptyParts))
    {
        m_normalized.reserve(m_words.size());
        for (const QString &word : std::as_const(m_words)) {
            m_normalized.append(normalized(word));
        }
    }

    QList<Action> run()
    {
        qsizetype index = 0;
        while (index < m_words.size()) {
            const Command *command = matchCommand(m_normalized, index);
            if (!command) {
                appendWord(m_words.at(index));
                ++index;
                continue;
            }
            const QString &lastWord = m_words.at(index + command->words.size() - 1);
            index += command->words.size();
            apply(*command, endsSentence(lastWord.back()));
        }
        flush();
        return m_actions;
    }

private:
    void appendWord(const QString &word)
    {
        if (m_capitalizeNext) {
            m_phrase.append(capitalized(word));
        } else if (m_lowercaseNext) {
            m_phrase.append(decapitalized(word));
        } else {
            m_phrase.append(word);
        }
        m_capitalizeNext = false;
        m_lowercaseNext = false;
    }

    void apply(const Command &command, bool modelEndedSentence)
    {
        m_capitalizeNext = false;
        m_lowercaseNext = false;
        switch (command.kind) {
        case CommandKind::Punctuation:
            punctuate(command.symbol, modelEndedSentence);
            break;
        case CommandKind::NewParagraph:
            flush();
            m_actions.append(Action {Action::NewLine, QString()});
            m_actions.append(Action {Action::NewLine, QString()});
            break;
        case CommandKind::NewLine:
            flush();
            m_actions.append(Action {Action::NewLine, QString()});
            break;
        case CommandKind::DeleteThat:
            deleteThat();
            break;
        }
    }

    void punctuate(QChar symbol, bool modelEndedSentence)
    {
        QString *target = nullptr;
        if (!m_phrase.isEmpty()) {
            target = &m_phrase.last();
        } else if (!m_actions.isEmpty() && m_actions.last().kind == Action::Insert) {
            target = &m_actions.last().text;
        }
        if (target) {
            chopTrailingPunctuation(*target);
            target->append(symbol);
        } else {
            m_actions.append(Action {Action::Insert, QString(symbol)});
        }
        m_capitalizeNext = endsSentence(symbol);
        m_lowercaseNext = !m_capitalizeNext && modelEndedSentence;
    }

    void deleteThat()
    {
        if (!m_phrase.isEmpty()) {
            m_phrase.clear();
        } else if (!m_actions.isEmpty() && m_actions.last().kind != Action::DeletePrevious) {
            m_actions.removeLast();
        } else {
            m_actions.append(Action {Action::DeletePrevious, QString()});
        }
    }

    void flush()
    {
        if (!m_phrase.isEmpty()) {
            m_actions.append(Action {Action::Insert, m_phrase.join(u' ')});
            m_phrase.clear();
        }
    }

    QStringList m_words;
    QStringList m_normalized;
    QStringList m_phrase;
    QList<Action> m_actions;
    bool m_capitalizeNext = false;
    bool m_lowercaseNext = false;
};
}

namespace TranscriptText
{
QList<Action> plain(const QString &transcript)
{
    const QString text = transcript.simplified();
    if (text.isEmpty()) {
        return {};
    }
    return {Action {Action::Insert, text}};
}

QList<Action> applyCommands(const QString &transcript)
{
    return CommandApplier(transcript).run();
}

QString fitToContext(const QString &text, const QString &textBeforeCursor)
{
    QString result = text.trimmed();
    if (result.isEmpty()) {
        return result;
    }
    if (isClosingPunctuation(result.front())) {
        return result;
    }
    qsizetype wordEnd = result.indexOf(u' ');
    wordEnd = wordEnd < 0 ? result.size() : wordEnd;
    const QString firstWord = result.left(wordEnd);
    result.replace(0, wordEnd, isSentenceStart(textBeforeCursor) ? capitalized(firstWord) : decapitalized(firstWord));

    if (textBeforeCursor.isEmpty() || textBeforeCursor.back().isSpace() || opensGroup(textBeforeCursor.back())) {
        return result;
    }
    return QLatin1Char(' ') + result;
}
}
