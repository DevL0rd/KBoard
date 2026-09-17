#pragma once

#include <QString>
#include <QStringList>

namespace TextAnalysis
{
enum class Casing
{
    Lower,
    Capitalised,
    Upper,
    Mixed,
};

struct WordContext
{
    QString word;
    QString wordBefore;
    QString wordAfter;
    bool protectedToken = false;
    bool midWord = false;
    bool sentenceStart = false;
    QStringList previousKeys;
};

WordContext analyze(const QString &textBeforeCursor, const QString &textAfterCursor);
bool isSentenceStart(const QString &textBefore);
QStringList previousWordKeys(const QString &textBefore, int count);

bool isApostrophe(QChar c);
bool isOpeningPunctuation(QChar c);
bool isClosingPunctuation(QChar c);
bool isSentenceEnd(QChar c);
bool isLineBreak(QChar c);
bool isWordChar(QChar c);
bool isPlainWord(const QString &word);
bool hasInnerUppercase(const QString &word);
QString normalizeApostrophes(const QString &word);
QString normalizeKey(const QString &word);
QString stripPunctuation(const QString &word);

Casing casingOf(const QString &word);
QString applyCasing(const QString &typed, const QString &candidate, bool sentenceStart);
QString capitalise(const QString &word);
}
