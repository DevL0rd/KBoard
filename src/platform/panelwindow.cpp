/*
    SPDX-FileCopyrightText: 2025 Devin Lin <devin@kde.org>
    SPDX-FileCopyrightText: 2026 Kristen McWilliam <kristen@kde.org>
    SPDX-FileCopyrightText: 2026 DevL0rd <dmhzmxn@gmail.com>

    SPDX-License-Identifier: GPL-3.0-only
*/

#include "panelwindow.h"
#include "inputpanelintegration.h"

#include <KWindowEffects>
#include <QRegion>

PanelWindow::PanelWindow(QWindow *parent)
    : QQuickWindow(parent)
{
    setFlag(Qt::FramelessWindowHint);
    setColor(Qt::transparent);
    connect(this, &QWindow::screenChanged, this, &PanelWindow::trackScreen);
    trackScreen(screen());
}

void PanelWindow::trackScreen(QScreen *screen)
{
    if (m_geometryConnection) {
        disconnect(m_geometryConnection);
    }
    m_screen = screen;
    if (screen) {
        m_geometryConnection = connect(screen, &QScreen::geometryChanged, this, &PanelWindow::screenGeometryChanged);
    }
    Q_EMIT screenGeometryChanged();
}

QVariantList PanelWindow::interactiveRects() const
{
    return m_interactiveRects;
}

void PanelWindow::setInteractiveRects(const QVariantList &rects)
{
    if (rects == m_interactiveRects) {
        return;
    }
    m_interactiveRects = rects;
    applyMask();
    Q_EMIT interactiveRectsChanged();
}

void PanelWindow::applyMask()
{
    QRegion region;
    for (const QVariant &value : std::as_const(m_interactiveRects)) {
        region += value.toRectF().toAlignedRect();
    }
    setMask(region);
}

QRectF PanelWindow::blurRect() const
{
    return m_blurRect;
}

void PanelWindow::setBlurRect(const QRectF &rect)
{
    if (rect == m_blurRect) {
        return;
    }
    m_blurRect = rect;
    applyBlur();
    Q_EMIT blurRectChanged();
}

bool PanelWindow::blurEnabled() const
{
    return m_blurEnabled;
}

void PanelWindow::setBlurEnabled(bool enabled)
{
    if (enabled == m_blurEnabled) {
        return;
    }
    m_blurEnabled = enabled;
    applyBlur();
    Q_EMIT blurEnabledChanged();
}

void PanelWindow::applyBlur()
{
    if (!handle()) {
        return;
    }
    KWindowEffects::enableBlurBehind(this, m_blurEnabled && !m_blurRect.isEmpty(), QRegion(m_blurRect.toAlignedRect()));
}

int PanelWindow::screenWidth() const
{
    return m_screen ? m_screen->geometry().width() : 0;
}

int PanelWindow::screenHeight() const
{
    return m_screen ? m_screen->geometry().height() : 0;
}

bool PanelWindow::isPortrait() const
{
    return screenHeight() > screenWidth();
}

bool PanelWindow::panelReady() const
{
    return m_panelReady;
}

bool PanelWindow::initPanel()
{
    if (m_panelReady) {
        return true;
    }
    m_panelReady = initInputPanelIntegration(this, InputPanelRole::Keyboard);
    if (m_panelReady) {
        applyMask();
        applyBlur();
        Q_EMIT panelReadyChanged();
    }
    return m_panelReady;
}
