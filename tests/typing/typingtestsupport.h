#pragma once

#include <QString>
#include <QStringList>
#include <QVariantList>

#include <memory>

class TypingEngine;

namespace TypingTest
{
constexpr double KeyWidth = 96.0;
constexpr double KeyHeight = 128.0;

void prepareEnvironment();
void resetSettings();
std::unique_ptr<TypingEngine> readyEngine();
QVariantList qwertyLayout();
QVariantList glidePath(const QString &word, quint32 seed);
void typeWord(TypingEngine &engine, QString &text, const QString &word);
}
