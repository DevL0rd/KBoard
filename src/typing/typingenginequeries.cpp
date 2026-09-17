#include "typingengine.h"

#include "autocorrectpolicy.h"
#include "kboardsettings.h"
#include "predictor.h"
#include "textexpansions.h"
#include "wordranker.h"
#include "wordscorer.h"

QString TypingEngine::correctionFor(const QString &word)
{
    const QString trimmed = word.trimmed();
    TextAnalysis::WordContext context = TextAnalysis::analyze(trimmed, QString());
    if (!m_data || context.protectedToken || context.word.isEmpty() || context.word != trimmed) {
        return QString();
    }
    context.previousKeys = QStringList {QStringLiteral("<s>")};
    return correctionWith(WordScorer(*m_data, m_store.model()), context);
}

QString TypingEngine::correctionWith(const WordScorer &scorer, const TextAnalysis::WordContext &context) const
{
    const WordRanker ranker(scorer, m_corrector);
    const WordQuery query = ranker.query(context.word, context.previousKeys);
    return AutocorrectPolicy(scorer, autocorrectStrength()).decide(query, ranker.rank(query));
}

QStringList TypingEngine::predictNext(const QString &textBeforeCursor)
{
    QStringList texts;
    if (!m_data || !m_policy.allowsSuggestions() || !KBoardSettings::self()->nextWordPrediction()) {
        return texts;
    }
    const QString tail = textBeforeCursor.right(256);
    const WordScorer scorer(*m_data, m_store.model());
    for (const Suggestion &suggestion :
        Predictor(scorer).predict(TextAnalysis::previousWordKeys(tail, 2), TextAnalysis::isSentenceStart(tail), 5)) {
        texts.append(suggestion.text);
    }
    return texts;
}

QStringList TypingEngine::completionsFor(const QString &prefix, int limit)
{
    QStringList texts;
    if (!m_data || prefix.trimmed().isEmpty()) {
        return texts;
    }
    const WordScorer scorer(*m_data, m_store.model());
    const WordRanker ranker(scorer, m_corrector);
    for (const Suggestion &suggestion : ranker.completions(ranker.query(prefix.trimmed(), {}))) {
        if (texts.size() >= limit) {
            break;
        }
        texts.append(suggestion.text);
    }
    return texts;
}

QString TypingEngine::expansionFor(const QString &word) const
{
    return expansionIn(KBoardSettings::self()->textExpansions(), word);
}

bool TypingEngine::isValidWord(const QString &word) const
{
    return m_data && WordScorer(*m_data, m_store.model()).isValid(TextAnalysis::stripPunctuation(word));
}
