/*
    SPDX-FileCopyrightText: 2025 Devin Lin <devin@kde.org>
    SPDX-FileCopyrightText: 2026 Kristen McWilliam <kristen@kde.org>
    SPDX-FileCopyrightText: 2026 DevL0rd <dmhzmxn@gmail.com>

    SPDX-License-Identifier: GPL-3.0-only
*/

#pragma once

#include <QPointer>
#include <QQuickWindow>
#include <QScreen>
#include <qqmlintegration.h>

class PanelWindow : public QQuickWindow
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QVariantList interactiveRects READ interactiveRects WRITE setInteractiveRects NOTIFY interactiveRectsChanged)
    Q_PROPERTY(QRectF blurRect READ blurRect WRITE setBlurRect NOTIFY blurRectChanged)
    Q_PROPERTY(bool blurEnabled READ blurEnabled WRITE setBlurEnabled NOTIFY blurEnabledChanged)
    Q_PROPERTY(int screenWidth READ screenWidth NOTIFY screenGeometryChanged)
    Q_PROPERTY(int screenHeight READ screenHeight NOTIFY screenGeometryChanged)
    Q_PROPERTY(bool portrait READ isPortrait NOTIFY screenGeometryChanged)
    Q_PROPERTY(bool panelReady READ panelReady NOTIFY panelReadyChanged)

public:
    explicit PanelWindow(QWindow *parent = nullptr);

    QVariantList interactiveRects() const;
    void setInteractiveRects(const QVariantList &rects);
    QRectF blurRect() const;
    void setBlurRect(const QRectF &rect);
    bool blurEnabled() const;
    void setBlurEnabled(bool enabled);
    int screenWidth() const;
    int screenHeight() const;
    bool isPortrait() const;
    bool panelReady() const;

    Q_INVOKABLE bool initPanel();

Q_SIGNALS:
    void interactiveRectsChanged();
    void blurRectChanged();
    void blurEnabledChanged();
    void screenGeometryChanged();
    void panelReadyChanged();

private:
    void trackScreen(QScreen *screen);
    void applyMask();
    void applyBlur();

    QVariantList m_interactiveRects;
    QRectF m_blurRect;
    bool m_blurEnabled = true;
    bool m_panelReady = false;
    QPointer<QScreen> m_screen;
    QMetaObject::Connection m_geometryConnection;
};
