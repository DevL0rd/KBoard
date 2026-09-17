#pragma once

#include "textanalysis.h"

struct InputPolicy
{
    int purpose = 0;
    int hint = 0;
    bool sensitive = false;

    bool allowsSuggestions() const;
    bool allowsAutocorrect() const;
    bool forcesUppercase() const;
    bool shouldCapitalize(const TextAnalysis::WordContext &context, bool autoCapitalize) const;

    bool operator==(const InputPolicy &other) const = default;
};
