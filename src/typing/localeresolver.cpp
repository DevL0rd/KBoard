#include "localeresolver.h"

#include "kboardpaths.h"

#include <QFileInfo>
#include <QHash>

namespace LocaleResolver
{
namespace
{
const QHash<QString, QString> &layoutLocales()
{
    static const QHash<QString, QString> locales = [] {
        const char *const pairs[][2] = {
            {"us", "en_US"},
            {"gb", "en_GB"},
            {"au", "en_AU"},
            {"ca", "en_CA"},
            {"ie", "en_IE"},
            {"nz", "en_NZ"},
            {"za", "en_ZA"},
            {"in", "en_IN"},
            {"de", "de_DE"},
            {"at", "de_AT"},
            {"ch", "de_CH"},
            {"fr", "fr_FR"},
            {"be", "fr_BE"},
            {"es", "es_ES"},
            {"latam", "es_MX"},
            {"it", "it_IT"},
            {"pt", "pt_PT"},
            {"br", "pt_BR"},
            {"nl", "nl_NL"},
            {"se", "sv_SE"},
            {"no", "nb_NO"},
            {"dk", "da_DK"},
            {"fi", "fi_FI"},
            {"pl", "pl_PL"},
            {"cz", "cs_CZ"},
            {"sk", "sk_SK"},
            {"hu", "hu_HU"},
            {"ro", "ro_RO"},
            {"tr", "tr_TR"},
            {"ru", "ru_RU"},
            {"ua", "uk_UA"},
            {"gr", "el_GR"},
        };
        QHash<QString, QString> map;
        for (const auto &pair : pairs) {
            map.insert(QString::fromLatin1(pair[0]), QString::fromLatin1(pair[1]));
        }
        return map;
    }();
    return locales;
}

bool isInstalled(const QString &locale)
{
    return QFileInfo::exists(hunspellFile(locale, QStringLiteral("dic")))
        && QFileInfo::exists(dictionaryFile(languageOf(locale), QStringLiteral("freq")));
}

QString preferredLocale(const QString &preferredLanguage, const QString &layoutLocale)
{
    const QString preferred = QString(preferredLanguage).replace(u'-', u'_');
    const QString language = preferred.section(u'_', 0, 0).toLower();
    if (preferred.contains(u'_')) {
        return language + u'_' + preferred.section(u'_', 1, 1).toUpper();
    }
    if (layoutLocale.startsWith(language + u'_')) {
        return QString();
    }
    return language + u'_' + language.toUpper();
}
}

QString localeForLayout(const QString &layout)
{
    QString id = layout.trimmed();
    if (id.size() == 5 && id.at(2) == u'_') {
        return id;
    }
    return layoutLocales().value(id.section(u'(', 0, 0).section(u'+', 0, 0).section(u'-', 0, 0).toLower());
}

QString resolve(const QString &layout, const QString &preferredLanguage)
{
    QString layoutLocale = localeForLayout(layout);
    if (preferredLanguage.isEmpty()) {
        return layoutLocale;
    }
    const QString candidate = preferredLocale(preferredLanguage, layoutLocale);
    return !candidate.isEmpty() && isInstalled(candidate) ? candidate : layoutLocale;
}

QString languageOf(const QString &locale)
{
    return locale.section(u'_', 0, 0);
}

QString hunspellFile(const QString &locale, const QString &suffix)
{
    return QStringLiteral("/usr/share/hunspell/%1.%2").arg(locale, suffix);
}

QString dictionaryFile(const QString &language, const QString &suffix)
{
    return KBoardPaths::dataFile(QStringLiteral("dictionaries/%1.%2").arg(language, suffix));
}
}
