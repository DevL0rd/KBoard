#pragma once

#include <QString>

namespace VoiceStates
{
inline const QString idle = QStringLiteral("idle");
inline const QString loading = QStringLiteral("loading");
inline const QString listening = QStringLiteral("listening");
inline const QString processing = QStringLiteral("processing");
inline const QString error = QStringLiteral("error");
inline const QString needsModel = QStringLiteral("needs-model");
}
