#include "typingengine.h"

#include "commitanalysis.h"
#include "kboardsettings.h"
#include "wordscorer.h"

#include <QDateTime>

namespace
{
constexpr int MaxLearnLength = 48;

qint64 now()
{
    return QDateTime::currentSecsSinceEpoch();
}
}

QString TypingEngine::lastCorrectionOriginal() const
{
    return m_lastOriginal;
}

QString TypingEngine::lastCorrectionReplacement() const
{
    return m_lastReplacement;
}

QStringList TypingEngine::learnedWords() const
{
    return m_store.model().knownWords();
}

bool TypingEngine::learningAllowed() const
{
    return m_data && KBoardSettings::self()->learnWords() && m_policy.allowsSuggestions();
}

bool TypingEngine::isLearned(const QString &word) const
{
    return m_store.model().isKnown(TextAnalysis::normalizeKey(TextAnalysis::stripPunctuation(word)));
}

void TypingEngine::clearLastCorrection()
{
    if (m_lastOriginal.isEmpty() && m_lastReplacement.isEmpty()) {
        return;
    }
    m_lastOriginal.clear();
    m_lastReplacement.clear();
    Q_EMIT lastCorrectionChanged();
}

void TypingEngine::updateLastCorrection()
{
    if (m_lastReplacement.isEmpty()) {
        return;
    }
    QString tail = m_before.right(m_lastReplacement.size() + m_lastOriginal.size() + 4);
    while (!tail.isEmpty() && (tail.back().isSpace() || tail.back().isPunct())) {
        tail.chop(1);
    }
    if (!tail.endsWith(m_lastReplacement) && !tail.endsWith(m_lastOriginal)) {
        clearLastCorrection();
    }
}

void TypingEngine::learnedChanged()
{
    m_store.markChanged();
    Q_EMIT learnedWordsChanged();
    recompute();
}

void TypingEngine::acceptWord(const QString &word)
{
    const QString stripped = TextAnalysis::stripPunctuation(word.trimmed());
    if (stripped.isEmpty()) {
        return;
    }
    const bool appliedCorrection = !m_autocorrection.isEmpty() && stripped == m_autocorrection && m_context.word != stripped;
    const QString original = m_context.word;
    clearLastCorrection();
    if (appliedCorrection) {
        m_lastOriginal = original;
        m_lastReplacement = stripped;
        Q_EMIT lastCorrectionChanged();
    }
    if (!learningAllowed()) {
        return;
    }
    const WordScorer scorer(*m_data, m_store.model());
    const auto committed = analyseCommit(scorer, m_before, m_context.wordBefore, stripped);
    if (!committed) {
        return;
    }
    UserModel &model = m_store.model();
    const bool wasKnown = model.isKnown(committed->key);
    model.learnWord(committed->form, 1, false, now());
    model.learnSequence(committed->previousKeys, committed->key);
    m_store.markChanged();
    if (wasKnown != model.isKnown(committed->key)) {
        Q_EMIT learnedWordsChanged();
    }
}

void TypingEngine::learn(const QString &word)
{
    QString form = TextAnalysis::stripPunctuation(word.trimmed());
    if (!m_data || form.isEmpty() || form.size() > MaxLearnLength) {
        return;
    }
    form = TextAnalysis::normalizeApostrophes(form);
    m_store.model().learnWord(form, UserModel::KnownThreshold, true, now());
    learnedChanged();
}

void TypingEngine::forget(const QString &word)
{
    const QString key = TextAnalysis::normalizeKey(TextAnalysis::stripPunctuation(word.trimmed()));
    if (!m_data || key.isEmpty()) {
        return;
    }
    m_store.model().forget(key);
    learnedChanged();
}

QString TypingEngine::revertCorrection()
{
    QString original = m_lastOriginal;
    const QString replacement = m_lastReplacement;
    if (original.isEmpty()) {
        return QString();
    }
    clearLastCorrection();
    if (!learningAllowed()) {
        recompute();
        return original;
    }
    UserModel &model = m_store.model();
    model.rejectCorrection(TextAnalysis::normalizeKey(original), TextAnalysis::normalizeKey(replacement));
    if (TextAnalysis::isPlainWord(original)) {
        model.learnWord(original, UserModel::KnownThreshold, false, now());
    }
    learnedChanged();
    return original;
}

void TypingEngine::clearLearned()
{
    m_store.model().clear();
    clearLastCorrection();
    learnedChanged();
    m_store.flush();
}

bool TypingEngine::exportLearned(const QString &filePath) const
{
    return m_store.exportWords(filePath);
}

int TypingEngine::importLearned(const QString &filePath)
{
    const int imported = m_store.importWords(filePath);
    if (imported > 0) {
        learnedChanged();
    }
    return imported;
}

void TypingEngine::flush()
{
    m_store.flush();
}
