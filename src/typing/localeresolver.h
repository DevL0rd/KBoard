#pragma once

#include <QString>

namespace LocaleResolver
{
QString localeForLayout(const QString &layout);
QString resolve(const QString &layout, const QString &preferredLanguage);
QString languageOf(const QString &locale);
QString hunspellFile(const QString &locale, const QString &suffix);
QString dictionaryFile(const QString &language, const QString &suffix);
}
