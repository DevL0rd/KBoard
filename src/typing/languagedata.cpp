#include "languagedata.h"

#include "localeresolver.h"
#include "textanalysis.h"

#include <QFile>
#include <QFileInfo>
#include <QStringEncoder>

#include <hunspell.hxx>

LanguageData::LanguageData() = default;
LanguageData::~LanguageData() = default;

bool LanguageData::spell(const QString &word) const
{
    if (word.isEmpty()) {
        return false;
    }
    const QString normalized = TextAnalysis::normalizeApostrophes(word);
    QStringEncoder encoder(encoding.constData());
    const QByteArray encoded = encoder.encode(normalized);
    if (encoder.hasError()) {
        return false;
    }
    return hunspell->spell(encoded.toStdString());
}

namespace
{
QString loadDictionaries(LanguageData &data)
{
    const QString affPath = LocaleResolver::hunspellFile(data.locale, QStringLiteral("aff"));
    const QString dicPath = LocaleResolver::hunspellFile(data.locale, QStringLiteral("dic"));
    if (!QFileInfo::exists(dicPath) || !QFileInfo::exists(affPath)) {
        return QStringLiteral("The Hunspell dictionary %1 is not installed (%2)").arg(data.locale, dicPath);
    }
    QString error;
    if (!data.lexicon.load(LocaleResolver::dictionaryFile(data.language, QStringLiteral("freq")), &error)) {
        return error;
    }
    const QString ngramPath = LocaleResolver::dictionaryFile(data.language, QStringLiteral("ngram"));
    if (QFileInfo::exists(ngramPath) && !data.ngrams.load(ngramPath, data.lexicon, &error)) {
        return error;
    }
    data.hunspell = std::make_unique<Hunspell>(QFile::encodeName(affPath).constData(), QFile::encodeName(dicPath).constData());
    data.encoding = QByteArray::fromStdString(data.hunspell->get_dict_encoding());
    if (!QStringEncoder(data.encoding.constData()).isValid()) {
        return QStringLiteral("The Hunspell dictionary %1 uses the unsupported encoding %2")
            .arg(data.locale, QString::fromLatin1(data.encoding));
    }
    return QString();
}
}

LanguageLoad loadLanguage(const LanguageSource &source)
{
    LanguageLoad result;
    auto data = std::make_shared<LanguageData>();
    data->language = source.language;
    data->locale = source.locale;
    result.error = loadDictionaries(*data);
    if (!result.error.isEmpty()) {
        return result;
    }
    QFile userFile(source.userModelPath);
    if (userFile.exists()) {
        if (!userFile.open(QIODevice::ReadOnly)) {
            result.error = QStringLiteral("Cannot read learned words %1: %2").arg(source.userModelPath, userFile.errorString());
            return result;
        }
        result.userData = userFile.readAll();
    }
    result.data = data;
    return result;
}
