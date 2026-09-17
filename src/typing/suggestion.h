#pragma once

#include <QString>

enum class SuggestionKind
{
    Typed,
    Correction,
    Completion,
    Prediction,
    Expansion,
};

struct Suggestion
{
    QString text;
    QString key;
    SuggestionKind kind = SuggestionKind::Typed;
    double score = 0;
    double cost = 0;
};

inline QString suggestionKindName(SuggestionKind kind)
{
    switch (kind) {
    case SuggestionKind::Typed:
        return QStringLiteral("typed");
    case SuggestionKind::Correction:
        return QStringLiteral("correction");
    case SuggestionKind::Completion:
        return QStringLiteral("completion");
    case SuggestionKind::Prediction:
        return QStringLiteral("prediction");
    case SuggestionKind::Expansion:
        return QStringLiteral("expansion");
    }
    return QString();
}
