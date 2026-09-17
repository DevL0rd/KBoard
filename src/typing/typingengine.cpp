#include "typingengine.h"

#include "autocorrectpolicy.h"
#include "inputcontext.h"
#include "kboardsettings.h"
#include "localeresolver.h"
#include "predictor.h"
#include "suggestionslots.h"
#include "wordranker.h"
#include "wordscorer.h"

TypingEngine::TypingEngine(QObject *parent)
    : QObject(parent)
    , m_geometry(KeyGeometry::qwerty())
    , m_corrector(m_geometry)
{
    connect(&m_loader, &LanguageLoader::loaded, this, &TypingEngine::applyLoad);
    connect(&m_loader, &LanguageLoader::loadingChanged, this, &TypingEngine::loadingChanged);
    connect(&m_glide, &GlideService::readyChanged, this, &TypingEngine::glideReadyChanged);
    connect(&m_glide, &GlideService::decoded, this, &TypingEngine::glideDecoded);

    KBoardSettings *settings = KBoardSettings::self();
    connect(settings, &KBoardSettings::activeLayoutChanged, this, &TypingEngine::startLoading);
    for (auto signal :
        {&KBoardSettings::suggestionsChanged, &KBoardSettings::autocorrectChanged, &KBoardSettings::nextWordPredictionChanged,
            &KBoardSettings::autoCapitalizeChanged, &KBoardSettings::textExpansionsChanged, &KBoardSettings::learnWordsChanged}) {
        connect(settings, signal, this, &TypingEngine::recompute);
    }
    startLoading();
}

TypingEngine::~TypingEngine() = default;

TypingEngine *TypingEngine::instance()
{
    static auto *engine = new TypingEngine;
    return engine;
}

TypingEngine *TypingEngine::create(QQmlEngine *, QJSEngine *)
{
    TypingEngine *engine = instance();
    QJSEngine::setObjectOwnership(engine, QJSEngine::CppOwnership);
    engine->attachInputContext(InputContext::instance());
    return engine;
}

void TypingEngine::attachInputContext(InputContext *context)
{
    auto sync = [this, context] { setPolicy(InputPolicy {context->contentPurpose(), context->contentHint(), context->isSensitive()}); };
    connect(context, &InputContext::contentTypeChanged, this, sync);
    connect(
        context, &InputContext::preferredLanguageChanged, this, [this, context] { setPreferredLanguage(context->preferredLanguage()); });
    sync();
    setPreferredLanguage(context->preferredLanguage());
}

QStringList TypingEngine::suggestions() const
{
    return m_suggestions;
}

QVariantList TypingEngine::suggestionItems() const
{
    return m_suggestionItems;
}

QString TypingEngine::autocorrection() const
{
    return m_autocorrection;
}

QString TypingEngine::currentWord() const
{
    return m_context.word;
}

int TypingEngine::wordCharsBefore() const
{
    return int(m_context.wordBefore.size());
}

int TypingEngine::wordCharsAfter() const
{
    return int(m_context.wordAfter.size());
}

bool TypingEngine::shouldCapitalize() const
{
    return m_shouldCapitalize;
}

QString TypingEngine::language() const
{
    return m_language;
}

bool TypingEngine::isReady() const
{
    return bool(m_data);
}

bool TypingEngine::isLoading() const
{
    return m_loader.isLoading();
}

QString TypingEngine::errorString() const
{
    return m_error;
}

int TypingEngine::contentPurpose() const
{
    return m_policy.purpose;
}

void TypingEngine::setContentPurpose(int purpose)
{
    setPolicy(InputPolicy {purpose, m_policy.hint, m_policy.sensitive});
}

int TypingEngine::contentHint() const
{
    return m_policy.hint;
}

void TypingEngine::setContentHint(int hint)
{
    setPolicy(InputPolicy {m_policy.purpose, hint, m_policy.sensitive});
}

bool TypingEngine::isSensitive() const
{
    return m_policy.sensitive;
}

void TypingEngine::setSensitive(bool sensitive)
{
    setPolicy(InputPolicy {m_policy.purpose, m_policy.hint, sensitive});
}

void TypingEngine::setPolicy(const InputPolicy &policy)
{
    if (m_policy == policy) {
        return;
    }
    m_policy = policy;
    Q_EMIT contentTypeChanged();
    recompute();
}

QString TypingEngine::preferredLanguage() const
{
    return m_preferredLanguage;
}

void TypingEngine::setPreferredLanguage(const QString &language)
{
    if (m_preferredLanguage == language) {
        return;
    }
    m_preferredLanguage = language;
    Q_EMIT preferredLanguageChanged();
    startLoading();
}

bool TypingEngine::isGlideReady() const
{
    return m_glide.isReady();
}

void TypingEngine::startLoading()
{
    const QString layout = KBoardSettings::self()->activeLayout();
    const QString locale = LocaleResolver::resolve(layout, m_preferredLanguage);
    if (locale.isEmpty()) {
        unloadLanguage();
        setError(QStringLiteral("No dictionary language is known for keyboard layout '%1'").arg(layout));
        return;
    }
    const bool alreadyLoaded = m_data && m_data->locale == locale && !m_loader.isLoading();
    if (alreadyLoaded || m_loader.pendingLocale() == locale) {
        return;
    }
    const QString language = LocaleResolver::languageOf(locale);
    m_store.flush();
    m_loader.load(LanguageSource {locale, language, UserStore::pathFor(language)});
}

void TypingEngine::applyLoad(const LanguageLoad &result)
{
    if (!result.data) {
        unloadLanguage();
        setError(result.error);
        return;
    }
    setError(m_store.open(result.data->language, result.userData));
    m_data = result.data;
    if (m_language != m_data->language) {
        m_language = m_data->language;
        Q_EMIT languageChanged();
    }
    Q_EMIT readyChanged();
    Q_EMIT learnedWordsChanged();
    m_glide.rebuild(m_data, m_geometry);
    recompute();
}

void TypingEngine::unloadLanguage()
{
    m_store.close();
    m_glide.reset();
    if (m_data) {
        m_data.reset();
        Q_EMIT readyChanged();
        Q_EMIT learnedWordsChanged();
    }
    recompute();
}

void TypingEngine::reload()
{
    unloadLanguage();
    startLoading();
}

void TypingEngine::setError(const QString &error)
{
    if (m_error == error) {
        return;
    }
    m_error = error;
    if (!error.isEmpty()) {
        qWarning("TypingEngine: %s", qPrintable(error));
    }
    Q_EMIT errorStringChanged();
}

int TypingEngine::autocorrectStrength() const
{
    return m_policy.allowsAutocorrect() ? KBoardSettings::self()->autocorrect() : int(AutocorrectPolicy::Off);
}

void TypingEngine::update(const QString &textBeforeCursor, const QString &textAfterCursor)
{
    m_before = textBeforeCursor;
    m_after = textAfterCursor;
    recompute();
}

void TypingEngine::recompute()
{
    updateContext();
    updateLastCorrection();
    updateSuggestions();
}

void TypingEngine::updateContext()
{
    const TextAnalysis::WordContext previous = m_context;
    m_context = TextAnalysis::analyze(m_before, m_after);
    if (previous.wordBefore != m_context.wordBefore || previous.wordAfter != m_context.wordAfter) {
        Q_EMIT currentWordChanged();
    }
    const bool capitalize = m_policy.shouldCapitalize(m_context, KBoardSettings::self()->autoCapitalize());
    if (capitalize != m_shouldCapitalize) {
        m_shouldCapitalize = capitalize;
        Q_EMIT shouldCapitalizeChanged();
    }
}

void TypingEngine::updateSuggestions()
{
    const KBoardSettings *settings = KBoardSettings::self();
    if (!m_data || !m_policy.allowsSuggestions() || m_context.protectedToken) {
        setSuggestionList({}, QString());
        return;
    }
    const WordScorer scorer(*m_data, m_store.model());
    if (m_context.word.isEmpty()) {
        const bool atGap = m_before.isEmpty() || m_before.back().isSpace() || TextAnalysis::isSentenceStart(m_before);
        const bool predict = settings->suggestions() && settings->nextWordPrediction() && atGap;
        const QVector<Suggestion> ranked
            = predict ? Predictor(scorer).predict(m_context.previousKeys, m_context.sentenceStart, 3) : QVector<Suggestion>();
        setSuggestionList(SuggestionSlots::forRanked(ranked), QString());
        return;
    }
    QString correction;
    const QVector<Suggestion> slots = wordSuggestions(scorer, &correction);
    setSuggestionList(settings->suggestions() ? slots : QVector<Suggestion>(), correction);
}

QVector<Suggestion> TypingEngine::wordSuggestions(const WordScorer &scorer, QString *autocorrection) const
{
    const WordRanker ranker(scorer, m_corrector);
    const WordQuery query = ranker.query(m_context.word, m_context.previousKeys);
    const QVector<Suggestion> ranked = ranker.rank(query);
    const QString expansion = expansionFor(m_context.word);
    if (expansion.isEmpty() && !m_context.midWord) {
        *autocorrection = AutocorrectPolicy(scorer, autocorrectStrength()).decide(query, ranked);
    }
    return SuggestionSlots::forWord(query, ranked, *autocorrection, expansion);
}

void TypingEngine::setSuggestionList(const QVector<Suggestion> &slots, const QString &autocorrection)
{
    QStringList texts;
    QVariantList items;
    for (const Suggestion &suggestion : slots) {
        texts.append(suggestion.text);
        items.append(QVariantMap {
            {QStringLiteral("text"), suggestion.text},
            {QStringLiteral("kind"), suggestionKindName(suggestion.kind)},
            {QStringLiteral("learned"), !suggestion.key.isEmpty() && m_store.model().isKnown(suggestion.key)},
        });
    }
    if (texts == m_suggestions && items == m_suggestionItems && autocorrection == m_autocorrection) {
        return;
    }
    m_suggestions = texts;
    m_suggestionItems = items;
    m_autocorrection = autocorrection;
    Q_EMIT suggestionsChanged();
}

void TypingEngine::setLayout(const QVariantList &keys)
{
    if (keys.isEmpty()) {
        return;
    }
    const KeyGeometry geometry = KeyGeometry::fromVariantList(keys);
    if (geometry.isEmpty()) {
        setError(QStringLiteral("The keyboard layout passed to TypingEngine.setLayout has no letter keys"));
        return;
    }
    setError(QString());
    if (geometry.fingerprint() == m_geometry.fingerprint() && m_glide.isReady()) {
        return;
    }
    m_geometry = geometry;
    m_corrector = Corrector(m_geometry);
    m_glide.rebuild(m_data, m_geometry);
    recompute();
}
