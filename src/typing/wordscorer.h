#pragma once

#include <QString>
#include <QStringList>

struct LanguageData;
class UserModel;

class WordScorer
{
public:
    WordScorer(const LanguageData &data, const UserModel &user);

    const LanguageData &data() const { return m_data; }
    const UserModel &user() const { return m_user; }

    double prior(const QString &key, int entry) const;
    double userPrior(const QString &key) const;
    double contextBoost(const QStringList &previousKeys, const QString &key, int entry) const;
    double contextualPrior(const QStringList &previousKeys, const QString &key, int entry) const;
    QString displayForm(const QString &key) const;
    bool isKnown(const QString &key) const;
    bool isValid(const QString &word) const;

    static constexpr double UserBaseline = 9.0;

private:
    double baseBoost(const QString &context, int entry, double weight) const;
    double userBoost(const QString &context, const QString &key) const;

    const LanguageData &m_data;
    const UserModel &m_user;
};
