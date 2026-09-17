/*
    SPDX-FileCopyrightText: 2026 Kristen McWilliam <kristen@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#pragma once

class QWindow;

#include "inputpanelrole.h"

bool initInputPanelIntegration(QWindow *window, InputPanelRole::Role role);
