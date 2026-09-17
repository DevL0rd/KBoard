/*
    SPDX-FileCopyrightText: 2026 Kristen McWilliam <kristen@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#pragma once

#include <QObject>
#include <qqmlintegration.h>

class InputPanelRole : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("InputPanelRole is an enum container and cannot be instantiated.")

public:
    enum Role {
        Keyboard = 0,
        OverlayPanel = 1,
    };
    Q_ENUM(Role)
};
