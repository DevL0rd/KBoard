#pragma once

#include "corrector.h"
#include "glideservice.h"
#include "inputpolicy.h"
#include "keygeometry.h"
#include "languageloader.h"
#include "suggestion.h"
#include "textanalysis.h"
#include "userstore.h"

#include <QObject>
#include <QQmlEngine>
#include <QStringList>
#include <QVariantList>

#include <memory>

class InputContext;
class WordScorer;

class TypingEngine : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QStringList suggestions READ suggestions NOTIFY suggestionsChanged)
    Q_PROPERTY(QVariantList suggestionItems READ suggestionItems NOTIFY suggestionsChanged)
    Q_PROPERTY(QString autocorrection READ autocorrection NOTIFY suggestionsChanged)
    Q_PROPERTY(QString currentWord READ currentWord NOTIFY currentWordChanged)
    Q_PROPERTY(int wordCharsBefore READ wordCharsBefore NOTIFY currentWordChanged)
    Q_PROPERTY(int wordCharsAfter READ wordCharsAfter NOTIFY currentWordChanged)
    Q_PROPERTY(bool shouldCapitalize READ shouldCapitalize NOTIFY shouldCapitalizeChanged)
    Q_PROPERTY(QString language READ language NOTIFY languageChanged)
    Q_PROPERTY(bool ready READ isReady NOTIFY readyChanged)
    Q_PROPERTY(bool loading READ isLoading NOTIFY loadingChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorStringChanged)
    Q_PROPERTY(QString lastCorrectionOriginal READ lastCorrectionOriginal NOTIFY lastCorrectionChanged)
    Q_PROPERTY(QString lastCorrectionReplacement READ lastCorrectionReplacement NOTIFY lastCorrectionChanged)
    Q_PROPERTY(QStringList learnedWords READ learnedWords NOTIFY learnedWordsChanged)
    Q_PROPERTY(int contentPurpose READ contentPurpose WRITE setContentPurpose NOTIFY contentTypeChanged)
    Q_PROPERTY(int contentHint READ contentHint WRITE setContentHint NOTIFY contentTypeChanged)
    Q_PROPERTY(bool sensitive READ isSensitive WRITE setSensitive NOTIFY contentTypeChanged)
    Q_PROPERTY(QString preferredLanguage READ preferredLanguage WRITE setPreferredLanguage NOTIFY preferredLanguageChanged)
    Q_PROPERTY(bool glideReady READ isGlideReady NOTIFY glideReadyChanged)

public:
    explicit TypingEngine(QObject *parent = nullptr);
    ~TypingEngine() override;

    static TypingEngine *instance();
    static TypingEngine *create(QQmlEngine *, QJSEngine *);

    void attachInputContext(InputContext *context);

    QStringList suggestions() const;
    QVariantList suggestionItems() const;
    QString autocorrection() const;
    QString currentWord() const;
    int wordCharsBefore() const;
    int wordCharsAfter() const;
    bool shouldCapitalize() const;
    QString language() const;
    bool isReady() const;
    bool isLoading() const;
    QString errorString() const;
    QString lastCorrectionOriginal() const;
    QString lastCorrectionReplacement() const;
    QStringList learnedWords() const;
    int contentPurpose() const;
    void setContentPurpose(int purpose);
    int contentHint() const;
    void setContentHint(int hint);
    bool isSensitive() const;
    void setSensitive(bool sensitive);
    QString preferredLanguage() const;
    void setPreferredLanguage(const QString &language);
    bool isGlideReady() const;

    Q_INVOKABLE void update(const QString &textBeforeCursor, const QString &textAfterCursor);
    Q_INVOKABLE void setLayout(const QVariantList &keys);
    Q_INVOKABLE void acceptWord(const QString &word);
    Q_INVOKABLE void forget(const QString &word);
    Q_INVOKABLE void learn(const QString &word);
    Q_INVOKABLE QString revertCorrection();
    Q_INVOKABLE QString correctionFor(const QString &word);
    Q_INVOKABLE QStringList predictNext(const QString &textBeforeCursor);
    Q_INVOKABLE QStringList completionsFor(const QString &prefix, int limit = 3);
    Q_INVOKABLE QStringList decodeGlide(const QVariantList &points);
    Q_INVOKABLE void decodeGlideAsync(const QVariantList &points);
    Q_INVOKABLE QString expansionFor(const QString &word) const;
    Q_INVOKABLE bool isValidWord(const QString &word) const;
    Q_INVOKABLE bool isLearned(const QString &word) const;
    Q_INVOKABLE void clearLearned();
    Q_INVOKABLE bool exportLearned(const QString &filePath) const;
    Q_INVOKABLE int importLearned(const QString &filePath);
    Q_INVOKABLE void reload();
    Q_INVOKABLE void flush();

Q_SIGNALS:
    void suggestionsChanged();
    void currentWordChanged();
    void shouldCapitalizeChanged();
    void languageChanged();
    void readyChanged();
    void loadingChanged();
    void errorStringChanged();
    void lastCorrectionChanged();
    void learnedWordsChanged();
    void contentTypeChanged();
    void preferredLanguageChanged();
    void glideReadyChanged();
    void glideDecoded(const QStringList &candidates);

private:
    void startLoading();
    void applyLoad(const LanguageLoad &result);
    void unloadLanguage();
    void setError(const QString &error);
    void setPolicy(const InputPolicy &policy);
    void recompute();
    void updateContext();
    void updateLastCorrection();
    void updateSuggestions();
    void setSuggestionList(const QVector<Suggestion> &slots, const QString &autocorrection);
    void clearLastCorrection();
    void learnedChanged();
    QVector<Suggestion> wordSuggestions(const WordScorer &scorer, QString *autocorrection) const;
    QString correctionWith(const WordScorer &scorer, const TextAnalysis::WordContext &context) const;
    GlideQuery glideQuery(const QVariantList &points) const;
    int autocorrectStrength() const;
    bool learningAllowed() const;

    std::shared_ptr<LanguageData> m_data;
    LanguageLoader m_loader;
    UserStore m_store;
    GlideService m_glide;
    KeyGeometry m_geometry;
    Corrector m_corrector;
    InputPolicy m_policy;

    QString m_before;
    QString m_after;
    TextAnalysis::WordContext m_context;
    QStringList m_suggestions;
    QVariantList m_suggestionItems;
    QString m_autocorrection;
    bool m_shouldCapitalize = false;
    QString m_language;
    QString m_error;
    QString m_lastOriginal;
    QString m_lastReplacement;
    QString m_preferredLanguage;
};
