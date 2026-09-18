/*
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "inputmethod_p.h"
InputMethod::InputMethod()
    : QWaylandClientExtensionTemplate<InputMethod>(1)
{ }

InputMethod::~InputMethod() = default;

void InputMethod::zwp_input_method_v1_activate(struct ::zwp_input_method_context_v1 *id)
{
    setCurrentContext(new InputMethodContext(id));
}

void InputMethod::zwp_input_method_v1_deactivate(struct ::zwp_input_method_context_v1 *context)
{
    Q_ASSERT(m_current->object() == context);
    setCurrentContext(nullptr);
}

void InputMethod::setCurrentContext(InputMethodContext *context)
{
    if (m_current.get() == context) {
        return;
    }
    m_current.reset(context);
    Q_EMIT activityChanged(m_current.use_count());

    if (m_current) {
        Q_EMIT activate();
    } else {
        Q_EMIT deactivate();
    }
}

InputMethodContext::InputMethodContext(struct ::zwp_input_method_context_v1 *id)
    : QtWayland::zwp_input_method_context_v1(id)
{ }

InputMethodContext::~InputMethodContext() = default;

void InputMethodContext::zwp_input_method_context_v1_reset()
{
    Q_EMIT reset();
}

void InputMethodContext::zwp_input_method_context_v1_commit_state(uint32_t serial)
{
    m_latestSerial = serial;
    Q_EMIT receivedCommit();
}

void InputMethodContext::zwp_input_method_context_v1_content_type(uint32_t hint, uint32_t purpose)
{
    m_contentHint = InputContext::ContentHint(hint);
    m_contentPurpose = InputContext::ContentPurpose(purpose);
    Q_EMIT contentTypeChanged();
}

void InputMethodContext::zwp_input_method_context_v1_preferred_language(const QString &language)
{
    Q_EMIT preferredLanguageChanged(language);
}

void InputMethodContext::zwp_input_method_context_v1_surrounding_text(const QString &text, uint32_t cursor, uint32_t anchor)
{
    m_text = text;
    m_cursor = cursor;
    m_anchor = anchor;
    Q_EMIT surroundingTextChanged(text, cursor, anchor);
}

void InputMethodContext::zwp_input_method_context_v1_invoke_action(uint32_t, uint32_t) { }

#include "moc_inputmethod_p.cpp"
