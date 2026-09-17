#include "commitanalysis.h"

#include "textanalysis.h"
#include "usermodel.h"
#include "wordscorer.h"

namespace
{
constexpr int MaxWordLength = 48;
constexpr int ContextWindow = 256;

bool isTrailingNoise(QChar c)
{
    return c.isSpace() || (c.isPunct() && c != u'\'' && c != u'’');
}

QString textBeforeWord(const QString &textBefore, const QString &partialWord, const QString &word)
{
    QString text = textBefore.right(ContextWindow);
    while (!text.isEmpty() && isTrailingNoise(text.back())) {
        text.chop(1);
    }
    if (TextAnalysis::normalizeKey(text.right(word.size())) == TextAnalysis::normalizeKey(word)) {
        text.chop(word.size());
    } else if (!partialWord.isEmpty() && text.endsWith(partialWord)) {
        text.chop(partialWord.size());
    }
    return text;
}

QString learnedForm(const WordScorer &scorer, const QString &word, bool sentenceStart)
{
    QString form = word;
    form = TextAnalysis::normalizeApostrophes(form);
    const QString key = TextAnalysis::normalizeKey(form);
    const TextAnalysis::Casing casing = TextAnalysis::casingOf(form);
    const bool existing = scorer.user().word(key) || scorer.isKnown(key);
    const bool shifted = sentenceStart && casing == TextAnalysis::Casing::Capitalised;
    if (existing && (casing == TextAnalysis::Casing::Upper || shifted)) {
        return scorer.displayForm(key);
    }
    return shifted ? key : form;
}
}

std::optional<CommittedWord> analyseCommit(
    const WordScorer &scorer, const QString &textBefore, const QString &partialWord, const QString &word)
{
    const QString stripped = TextAnalysis::stripPunctuation(word.trimmed());
    if (!TextAnalysis::isPlainWord(stripped) || stripped.size() > MaxWordLength) {
        return std::nullopt;
    }
    const QString context = textBeforeWord(textBefore, partialWord, stripped);
    CommittedWord committed;
    committed.form = learnedForm(scorer, stripped, TextAnalysis::isSentenceStart(context));
    committed.key = TextAnalysis::normalizeKey(stripped);
    committed.previousKeys = TextAnalysis::previousWordKeys(context, 2);
    return committed;
}
