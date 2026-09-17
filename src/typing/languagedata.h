#pragma once

#include "lexicon.h"

#include <QByteArray>
#include <QString>

#include <memory>

class Hunspell;

struct LanguageData
{
    LanguageData();
    ~LanguageData();

    QString language;
    QString locale;
    Lexicon lexicon;
    NGramModel ngrams;
    std::unique_ptr<Hunspell> hunspell;
    QByteArray encoding;

    bool spell(const QString &word) const;
};

struct LanguageSource
{
    QString locale;
    QString language;
    QString userModelPath;
};

struct LanguageLoad
{
    std::shared_ptr<LanguageData> data;
    QByteArray userData;
    QString error;
};

LanguageLoad loadLanguage(const LanguageSource &source);
