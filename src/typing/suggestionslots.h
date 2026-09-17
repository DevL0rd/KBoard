#pragma once

#include "suggestion.h"

#include <QVector>

struct WordQuery;

namespace SuggestionSlots
{
QVector<Suggestion> forWord(
    const WordQuery &query, const QVector<Suggestion> &ranked, const QString &autocorrection, const QString &expansion);
QVector<Suggestion> forRanked(const QVector<Suggestion> &ranked);
}
