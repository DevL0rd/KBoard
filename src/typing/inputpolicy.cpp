#include "inputpolicy.h"

#include "inputcontext.h"

namespace
{
bool purposeBlocksText(int purpose)
{
    switch (purpose) {
    case InputContext::content_purpose_digits:
    case InputContext::content_purpose_number:
    case InputContext::content_purpose_phone:
    case InputContext::content_purpose_url:
    case InputContext::content_purpose_email:
    case InputContext::content_purpose_password:
    case InputContext::content_purpose_date:
    case InputContext::content_purpose_time:
    case InputContext::content_purpose_datetime:
    case InputContext::content_purpose_terminal:
        return true;
    default:
        return false;
    }
}
}

bool InputPolicy::allowsSuggestions() const
{
    return !sensitive && !purposeBlocksText(purpose);
}

bool InputPolicy::allowsAutocorrect() const
{
    return allowsSuggestions() && purpose != InputContext::content_purpose_name;
}

bool InputPolicy::forcesUppercase() const
{
    return hint & InputContext::content_hint_uppercase;
}

bool InputPolicy::shouldCapitalize(const TextAnalysis::WordContext &context, bool autoCapitalize) const
{
    if (purposeBlocksText(purpose) || (hint & InputContext::content_hint_lowercase)) {
        return false;
    }
    if (forcesUppercase()) {
        return true;
    }
    if (!autoCapitalize || !context.wordBefore.isEmpty()) {
        return false;
    }
    const bool titleCase = (hint & InputContext::content_hint_titlecase) || purpose == InputContext::content_purpose_name;
    return titleCase || context.sentenceStart;
}
