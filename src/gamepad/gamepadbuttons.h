#pragma once

#include <QString>
#include <QStringList>

namespace GamepadButtons
{
QStringList names();
QString nameForSdlButton(int sdlButton);
int sdlButtonForName(const QString &name);
QString canonicalName(const QString &nameOrAlias);
bool isKnown(const QString &name);
QStringList controllerTypes();
QString glyph(const QString &controllerType, const QString &button);
QString typeForSdl(int sdlType, quint16 vendor, quint16 product);
bool isValveController(quint16 vendor, quint16 product);
}
