#include "textanalysis.h"

namespace TextAnalysis
{
bool isApostrophe(QChar c)
{
    return c == u'\'' || c == QChar(0x2019);
}

bool isOpeningPunctuation(QChar c)
{
    return QStringView(u"([{\"'*_<“‘¿¡").contains(c);
}

bool isClosingPunctuation(QChar c)
{
    return QStringView(u")]}\"'”’").contains(c);
}

bool isSentenceEnd(QChar c)
{
    return QStringView(u".!?…").contains(c);
}

bool isLineBreak(QChar c)
{
    return c == u'\n' || c == QChar::ParagraphSeparator || c == QChar::LineSeparator;
}

bool isWordChar(QChar c)
{
    return c.isLetterOrNumber() || c.isMark() || isApostrophe(c) || c == u'_';
}

bool isPlainWord(const QString &word)
{
    if (word.isEmpty() || !word.at(0).isLetter()) {
        return false;
    }
    return std::all_of(word.cbegin(), word.cend(), [](QChar c) { return c.isLetter() || c.isMark() || isApostrophe(c) || c == u'-'; });
}

bool hasInnerUppercase(const QString &word)
{
    bool seenLower = false;
    for (qsizetype i = 0; i < word.size(); ++i) {
        const QChar c = word.at(i);
        if (c.isUpper() && i > 0 && (seenLower || word.at(0).isLower())) {
            return true;
        }
        seenLower = seenLower || c.isLower();
    }
    return false;
}

QString normalizeApostrophes(const QString &word)
{
    QString normalized = word;
    normalized.replace(QChar(0x2019), u'\'');
    return normalized;
}

QString normalizeKey(const QString &word)
{
    return normalizeApostrophes(word.toLower());
}

QString stripPunctuation(const QString &word)
{
    qsizetype start = 0;
    qsizetype end = word.size();
    while (start < end && !word.at(start).isLetterOrNumber()) {
        ++start;
    }
    while (end > start && !word.at(end - 1).isLetterOrNumber()) {
        --end;
    }
    return word.mid(start, end - start);
}
}
