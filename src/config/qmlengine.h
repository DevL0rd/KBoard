#pragma once

#include <QString>

class QQmlApplicationEngine;

namespace KBoardQml
{
void loadModule(QQmlApplicationEngine &engine, const QString &uri, const QString &type);
}
