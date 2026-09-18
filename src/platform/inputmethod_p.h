/*
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#pragma once

#include <QObject>
#include <QString>
#include <QtWaylandClient/QWaylandClientExtensionTemplate>

#include <memory>
#include <qwayland-input-method-unstable-v1.h>
#include <qwayland-wayland.h>

#include "inputcontext.h"

class InputMethodContext;
class InputMethod : public QWaylandClientExtensionTemplate<InputMethod>, public QtWayland::zwp_input_method_v1
{
    Q_OBJECT
public:
    explicit InputMethod();
    ~InputMethod() override;

    std::shared_ptr<InputMethodContext> current() const { return m_current; }

Q_SIGNALS:
    void activate();
    void deactivate();
    void activityChanged(bool active);

private:
    void zwp_input_method_v1_activate(struct ::zwp_input_method_context_v1 *id) override;
    void zwp_input_method_v1_deactivate(struct ::zwp_input_method_context_v1 *context) override;

    void setCurrentContext(InputMethodContext *context);
    std::shared_ptr<InputMethodContext> m_current;
};

class InputMethodContext : public QObject, public QtWayland::zwp_input_method_context_v1
{
    Q_OBJECT
public:
    explicit InputMethodContext(struct ::zwp_input_method_context_v1 *id);
    ~InputMethodContext() override;

    QString m_text;
    uint32_t m_cursor = 0;
    uint32_t m_anchor = 0;
    uint32_t m_latestSerial = 0;
    InputContext::ContentHint m_contentHint = InputContext::content_hint_none;
    InputContext::ContentPurpose m_contentPurpose = InputContext::content_purpose_normal;

Q_SIGNALS:
    void reset();
    void contentTypeChanged();
    void preferredLanguageChanged(const QString &language);
    void surroundingTextChanged(const QString &surroundingText, uint32_t cursor, uint32_t anchor);
    void receivedCommit();

private:
    void zwp_input_method_context_v1_surrounding_text(const QString &text, uint32_t cursor, uint32_t anchor) override;
    void zwp_input_method_context_v1_reset() override;
    void zwp_input_method_context_v1_content_type(uint32_t hint, uint32_t purpose) override;
    void zwp_input_method_context_v1_invoke_action(uint32_t button, uint32_t index) override;
    void zwp_input_method_context_v1_commit_state(uint32_t serial) override;
    void zwp_input_method_context_v1_preferred_language(const QString &language) override;
};
