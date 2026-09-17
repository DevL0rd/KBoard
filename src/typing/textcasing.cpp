#include "textanalysis.h"

namespace TextAnalysis
{
Casing casingOf(const QString &word)
{
    const bool anyLower = std::any_of(word.cbegin(), word.cend(), [](QChar c) { return c.isLower(); });
    const bool anyUpper = std::any_of(word.cbegin(), word.cend(), [](QChar c) { return c.isUpper(); });
    if (!anyUpper) {
        return Casing::Lower;
    }
    if (!anyLower) {
        return word.size() == 1 ? Casing::Capitalised : Casing::Upper;
    }
    const bool restUpper = std::any_of(word.cbegin() + 1, word.cend(), [](QChar c) { return c.isUpper(); });
    return word.at(0).isUpper() && !restUpper ? Casing::Capitalised : Casing::Mixed;
}

QString capitalise(const QString &word)
{
    return word.isEmpty() ? word : word.left(1).toUpper() + word.mid(1);
}

QString applyCasing(const QString &typed, const QString &candidate, bool sentenceStart)
{
    const Casing casing = casingOf(typed);
    const auto letters = std::count_if(typed.cbegin(), typed.cend(), [](QChar c) { return c.isLetter(); });
    if (casing == Casing::Upper && letters >= 2) {
        return candidate.toUpper();
    }
    const bool capitalised = casing == Casing::Capitalised || casing == Casing::Upper;
    return capitalised || (typed.isEmpty() && sentenceStart) ? capitalise(candidate) : candidate;
}
}
