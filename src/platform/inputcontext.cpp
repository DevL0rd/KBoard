/*
    SPDX-FileCopyrightText: 2024 Aleix Pol i Gonzalez <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2024 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2026 DevL0rd <dmhzmxn@gmail.com>

    SPDX-License-Identifier: GPL-3.0-only
*/

#include "inputcontext.h"
#include "inputmethod_p.h"
#include "keycodes.h"

#include <QElapsedTimer>
#include <QRegularExpression>

#include <xkbcommon/xkbcommon-keysyms.h>

namespace
{
QElapsedTimer &eventClock()
{
    static QElapsedTimer timer;
    if (!timer.isValid()) {
        timer.start();
    }
    return timer;
}
}

InputContext::InputContext(QObject *parent)
    : QObject(parent)
    , m_inputMethod(new InputMethod)
{
    m_inputMethod->setParent(this);
    connect(m_inputMethod, &InputMethod::activityChanged, this, &InputContext::attach);
}

InputContext *InputContext::instance()
{
    static InputContext *context = new InputContext;
    return context;
}

InputContext *InputContext::create(QQmlEngine *, QJSEngine *)
{
    QJSEngine::setObjectOwnership(instance(), QJSEngine::CppOwnership);
    return instance();
}

void InputContext::attach()
{
    if (m_context) {
        disconnect(m_context.get(), nullptr, this, nullptr);
    }
    m_context = m_inputMethod->current();
    m_preedit.clear();
    Q_EMIT activeChanged();
    Q_EMIT preeditChanged();
    Q_EMIT surroundingTextChanged();
    Q_EMIT contentTypeChanged();
    if (!m_context) {
        Q_EMIT deactivated();
        return;
    }
    connect(m_context.get(), &InputMethodContext::surroundingTextChanged, this, &InputContext::surroundingTextChanged);
    connect(m_context.get(), &InputMethodContext::contentTypeChanged, this, &InputContext::contentTypeChanged);
    connect(m_context.get(), &InputMethodContext::preferredLanguageChanged, this, [this](const QString &language) {
        m_language = language;
        Q_EMIT preferredLanguageChanged();
    });
    connect(m_context.get(), &InputMethodContext::reset, this, [this] {
        m_preedit.clear();
        Q_EMIT preeditChanged();
        Q_EMIT resetRequested();
    });
    Q_EMIT activated();
}

bool InputContext::isActive() const
{
    return bool(m_context);
}

QString InputContext::surroundingText() const
{
    return m_context ? m_context->m_text : QString();
}

int InputContext::byteOffsetToChar(uint32_t bytes) const
{
    if (!m_context) {
        return 0;
    }
    const QByteArray utf8 = m_context->m_text.toUtf8();
    return QString::fromUtf8(utf8.left(qMin<qsizetype>(bytes, utf8.size()))).size();
}

int InputContext::cursorPosition() const
{
    return m_context ? byteOffsetToChar(m_context->m_cursor) : 0;
}

int InputContext::anchorPosition() const
{
    return m_context ? byteOffsetToChar(m_context->m_anchor) : 0;
}

QString InputContext::textBeforeCursor() const
{
    return surroundingText().left(cursorPosition());
}

QString InputContext::textAfterCursor() const
{
    return surroundingText().mid(cursorPosition());
}

bool InputContext::hasSurroundingText() const
{
    return m_context && !m_context->m_text.isEmpty();
}

int InputContext::contentHint() const
{
    return m_context ? int(m_context->m_contentHint) : int(content_hint_none);
}

int InputContext::contentPurpose() const
{
    return m_context ? int(m_context->m_contentPurpose) : int(content_purpose_normal);
}

bool InputContext::isSensitive() const
{
    const int hint = contentHint();
    return (hint & content_hint_sensitive_data) || (hint & content_hint_hidden_text) || contentPurpose() == content_purpose_password;
}

QString InputContext::preferredLanguage() const
{
    return m_language;
}

QString InputContext::preedit() const
{
    return m_preedit;
}

void InputContext::commit(const QString &text)
{
    if (!m_context) {
        return;
    }
    m_preedit.clear();
    m_context->commit_string(m_context->m_latestSerial, text);
    Q_EMIT preeditChanged();
    Q_EMIT committed(text);
}

void InputContext::setPreedit(const QString &text, int cursor)
{
    if (!m_context) {
        return;
    }
    m_preedit = text;
    const int chars = cursor < 0 ? text.size() : qMin(cursor, int(text.size()));
    m_context->preedit_styling(0, text.toUtf8().size(), 3);
    m_context->preedit_cursor(text.left(chars).toUtf8().size());
    m_context->preedit_string(m_context->m_latestSerial, text, text);
    Q_EMIT preeditChanged();
}

void InputContext::commitPreedit()
{
    if (m_preedit.isEmpty()) {
        return;
    }
    commit(m_preedit);
}

void InputContext::clearPreedit()
{
    if (!m_context || m_preedit.isEmpty()) {
        return;
    }
    m_preedit.clear();
    m_context->preedit_cursor(0);
    m_context->preedit_string(m_context->m_latestSerial, QString(), QString());
    Q_EMIT preeditChanged();
}

void InputContext::deleteSurrounding(int charsBefore, int charsAfter)
{
    if (!m_context) {
        return;
    }
    const QString text = surroundingText();
    const int cursor = cursorPosition();
    const int start = qMax(0, cursor - charsBefore);
    const int end = qMin(int(text.size()), cursor + charsAfter);
    const int bytesBefore = text.mid(start, cursor - start).toUtf8().size();
    const int bytesAfter = text.mid(cursor, end - cursor).toUtf8().size();
    if (bytesBefore == 0 && bytesAfter == 0) {
        return;
    }
    m_context->delete_surrounding_text(-bytesBefore, bytesBefore + bytesAfter);
    m_context->commit_string(m_context->m_latestSerial, QString());
}

void InputContext::backspace()
{
    if (!m_context) {
        return;
    }
    if (!m_preedit.isEmpty()) {
        setPreedit(m_preedit.chopped(1));
        if (m_preedit.isEmpty()) {
            clearPreedit();
        }
        return;
    }
    keysym(XKB_KEY_BackSpace);
}

void InputContext::deleteWordBefore()
{
    if (!m_context) {
        return;
    }
    if (hasSurroundingText()) {
        const QString before = textBeforeCursor();
        static const QRegularExpression word(QStringLiteral("(\\S+\\s*|\\s+)$"));
        const auto match = word.match(before);
        if (match.hasMatch()) {
            deleteSurrounding(match.capturedLength(), 0);
            return;
        }
    }
    shortcut(QStringLiteral("ctrl+backspace"));
}

void InputContext::enter()
{
    keysym(XKB_KEY_Return);
}

void InputContext::tab()
{
    keysym(XKB_KEY_Tab);
}

void InputContext::keysym(uint sym, uint modifiers)
{
    if (!m_context) {
        return;
    }
    const uint32_t time = eventClock().elapsed();
    m_context->keysym(m_context->m_latestSerial, time, sym, 1, modifiers);
    m_context->keysym(m_context->m_latestSerial, time, sym, 0, modifiers);
}

void InputContext::sendModifiers(uint modifiers)
{
    m_context->modifiers(m_context->m_latestSerial, modifiers, 0, 0, 0);
}

void InputContext::key(uint evdevCode, uint modifiers)
{
    if (!m_context) {
        return;
    }
    keyDown(evdevCode, modifiers);
    keyUp(evdevCode);
    if (modifiers) {
        sendModifiers(0);
    }
}

void InputContext::keyDown(uint evdevCode, uint modifiers)
{
    if (!m_context) {
        return;
    }
    if (modifiers) {
        sendModifiers(modifiers);
    }
    m_context->key(m_context->m_latestSerial, eventClock().elapsed(), evdevCode, 1);
}

void InputContext::keyUp(uint evdevCode)
{
    if (!m_context) {
        return;
    }
    m_context->key(m_context->m_latestSerial, eventClock().elapsed(), evdevCode, 0);
}

bool InputContext::shortcut(const QString &combo)
{
    uint modifiers = 0;
    uint code = 0;
    const auto parts = combo.toLower().split(QLatin1Char('+'), Qt::SkipEmptyParts);
    for (const QString &part : parts) {
        const QString name = part.trimmed();
        if (uint modifier = KeyCodes::modifierForName(name)) {
            modifiers |= modifier;
        } else {
            code = KeyCodes::evdevForName(name);
        }
    }
    if (!code || !m_context) {
        return false;
    }
    key(code, modifiers);
    return true;
}

void InputContext::moveCursor(int steps)
{
    const uint sym = steps < 0 ? XKB_KEY_Left : XKB_KEY_Right;
    for (int i = 0; i < qAbs(steps); ++i) {
        keysym(sym);
    }
}

void InputContext::moveCursorVertical(int steps)
{
    const uint sym = steps < 0 ? XKB_KEY_Up : XKB_KEY_Down;
    for (int i = 0; i < qAbs(steps); ++i) {
        keysym(sym);
    }
}
