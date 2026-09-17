#include "textanalysis.h"

#include <QSet>

namespace TextAnalysis
{
namespace
{
constexpr int ContextWindow = 256;
constexpr int AfterWindow = 64;
constexpr int ShortDottedToken = 5;

const QSet<QString> &abbreviations()
{
    static const QSet<QString> set = [] {
        QSet<QString> words;
        for (const char *word : {"mr", "mrs", "ms", "dr", "prof", "st", "vs", "e.g", "i.e", "jr", "sr", "approx", "no", "fig", "cf"}) {
            words.insert(QString::fromLatin1(word));
        }
        return words;
    }();
    return set;
}

qsizetype skipBackWhile(const QString &text, qsizetype end, bool (*predicate)(QChar))
{
    while (end > 0 && predicate(text.at(end - 1))) {
        --end;
    }
    return end;
}

bool isSpace(QChar c)
{
    return c.isSpace();
}

bool isNotSpace(QChar c)
{
    return !c.isSpace();
}

bool isOpeningNotUnderscore(QChar c)
{
    return isOpeningPunctuation(c) && c != u'_';
}

bool isNotWordChar(QChar c)
{
    return !isWordChar(c);
}

bool isTrailingAllowed(QChar c)
{
    return isClosingPunctuation(c) || isSentenceEnd(c) || c == u',' || c == u';' || c == u':';
}

bool containsLineBreak(QStringView text)
{
    return std::any_of(text.cbegin(), text.cend(), isLineBreak);
}

QString leadingWhile(const QString &text, bool (*predicate)(QChar))
{
    qsizetype end = 0;
    while (end < text.size() && predicate(text.at(end))) {
        ++end;
    }
    return text.left(end);
}

qsizetype trailingWordStart(const QString &token)
{
    qsizetype start = skipBackWhile(token, token.size(), isWordChar);
    while (start < token.size() && isApostrophe(token.at(start))) {
        ++start;
    }
    return start;
}

QString leadingWord(const QString &after)
{
    QString word = leadingWhile(after, isWordChar);
    while (!word.isEmpty() && isApostrophe(word.back())) {
        word.chop(1);
    }
    return word;
}

bool isProtected(const QString &prefix, const QString &suffix, const QString &word)
{
    const bool prefixProtected = !std::all_of(prefix.cbegin(), prefix.cend(), isOpeningPunctuation);
    const bool suffixProtected = !std::all_of(suffix.cbegin(), suffix.cend(), isTrailingAllowed);
    const bool codeLike = std::any_of(word.cbegin(), word.cend(), [](QChar c) { return c.isDigit() || c == u'_'; });
    return prefixProtected || suffixProtected || codeLike || hasInnerUppercase(word);
}

bool isAbbreviation(QString token)
{
    token = token.toLower();
    if (token.endsWith(QStringLiteral(".."))) {
        return false;
    }
    while (!token.isEmpty() && isSentenceEnd(token.back())) {
        token.chop(1);
    }
    while (!token.isEmpty() && isOpeningPunctuation(token.front())) {
        token.remove(0, 1);
    }
    return abbreviations().contains(token) || (token.contains(u'.') && token.size() <= ShortDottedToken);
}

bool endsSentence(const QString &token)
{
    for (qsizetype i = token.size() - 1; i >= 0 && !isWordChar(token.at(i)); --i) {
        if (isSentenceEnd(token.at(i))) {
            return true;
        }
    }
    return false;
}
}

WordContext analyze(const QString &textBeforeCursor, const QString &textAfterCursor)
{
    const QString before = textBeforeCursor.right(ContextWindow);
    const QString after = textAfterCursor.left(AfterWindow);
    const qsizetype tokenStart = skipBackWhile(before, before.size(), isNotSpace);
    const QString token = before.mid(tokenStart);
    const qsizetype wordStart = trailingWordStart(token);

    WordContext context;
    context.wordBefore = token.mid(wordStart);
    context.wordAfter = leadingWord(after);
    context.midWord = !context.wordAfter.isEmpty();
    context.word = context.wordBefore + context.wordAfter;
    const QString prefix = token.left(wordStart);
    const QString suffix = leadingWhile(after.mid(context.wordAfter.size()), isNotSpace);
    context.protectedToken = isProtected(prefix, suffix, context.word);
    const QString textBeforeWord = before.left(tokenStart) + prefix;
    context.sentenceStart = isSentenceStart(textBeforeWord);
    context.previousKeys = previousWordKeys(textBeforeWord, 2);
    return context;
}

bool isSentenceStart(const QString &textBefore)
{
    const qsizetype openingStart = skipBackWhile(textBefore, textBefore.size(), isOpeningNotUnderscore);
    const qsizetype end = skipBackWhile(textBefore, openingStart, isSpace);
    if (end == 0 || containsLineBreak(QStringView(textBefore).sliced(end, openingStart - end))) {
        return true;
    }
    const qsizetype closing = skipBackWhile(textBefore, end, isClosingPunctuation);
    if (closing == 0 || !isSentenceEnd(textBefore.at(closing - 1))) {
        return false;
    }
    if (closing != end && textBefore.at(closing - 1) != u'.') {
        return true;
    }
    const qsizetype tokenStart = skipBackWhile(textBefore, closing, isNotSpace);
    return !isAbbreviation(textBefore.mid(tokenStart, closing - tokenStart));
}

QStringList previousWordKeys(const QString &textBefore, int count)
{
    QStringList keys;
    qsizetype position = textBefore.size();
    while (keys.size() < count) {
        const qsizetype end = skipBackWhile(textBefore, position, isSpace);
        const qsizetype start = skipBackWhile(textBefore, end, isNotSpace);
        const QString token = textBefore.mid(start, end - start);
        const bool boundary = end == 0 || containsLineBreak(QStringView(textBefore).sliced(end, position - end));
        if (boundary || (endsSentence(token) && isSentenceStart(textBefore.left(end)))) {
            keys.prepend(QStringLiteral("<s>"));
            break;
        }
        const qsizetype wordEnd = skipBackWhile(token, token.size(), isNotWordChar);
        const QString word = token.left(wordEnd).mid(leadingWhile(token.left(wordEnd), isNotWordChar).size());
        if (!word.isEmpty()) {
            keys.prepend(normalizeKey(word));
        }
        position = start;
    }
    return keys;
}
}
