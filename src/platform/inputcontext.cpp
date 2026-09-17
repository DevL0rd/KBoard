/*
    SPDX-FileCopyrightText: 2024 Aleix Pol i Gonzalez <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2024 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2026 DevL0rd <dmhzmxn@gmail.com>

    SPDX-License-Identifier: GPL-3.0-only
*/

#include "inputcontext.h"
#include "inputmethod_p.h"

#include <QElapsedTimer>
#include <QRegularExpression>

#include <linux/input-event-codes.h>
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

uint evdevForName(const QString &name)
{
    static const QHash<QString, uint> names = {
        {QStringLiteral("a"), KEY_A},
        {QStringLiteral("b"), KEY_B},
        {QStringLiteral("c"), KEY_C},
        {QStringLiteral("d"), KEY_D},
        {QStringLiteral("e"), KEY_E},
        {QStringLiteral("f"), KEY_F},
        {QStringLiteral("g"), KEY_G},
        {QStringLiteral("h"), KEY_H},
        {QStringLiteral("i"), KEY_I},
        {QStringLiteral("j"), KEY_J},
        {QStringLiteral("k"), KEY_K},
        {QStringLiteral("l"), KEY_L},
        {QStringLiteral("m"), KEY_M},
        {QStringLiteral("n"), KEY_N},
        {QStringLiteral("o"), KEY_O},
        {QStringLiteral("p"), KEY_P},
        {QStringLiteral("q"), KEY_Q},
        {QStringLiteral("r"), KEY_R},
        {QStringLiteral("s"), KEY_S},
        {QStringLiteral("t"), KEY_T},
        {QStringLiteral("u"), KEY_U},
        {QStringLiteral("v"), KEY_V},
        {QStringLiteral("w"), KEY_W},
        {QStringLiteral("x"), KEY_X},
        {QStringLiteral("y"), KEY_Y},
        {QStringLiteral("z"), KEY_Z},
        {QStringLiteral("0"), KEY_0},
        {QStringLiteral("1"), KEY_1},
        {QStringLiteral("2"), KEY_2},
        {QStringLiteral("3"), KEY_3},
        {QStringLiteral("4"), KEY_4},
        {QStringLiteral("5"), KEY_5},
        {QStringLiteral("6"), KEY_6},
        {QStringLiteral("7"), KEY_7},
        {QStringLiteral("8"), KEY_8},
        {QStringLiteral("9"), KEY_9},
        {QStringLiteral("esc"), KEY_ESC},
        {QStringLiteral("escape"), KEY_ESC},
        {QStringLiteral("tab"), KEY_TAB},
        {QStringLiteral("enter"), KEY_ENTER},
        {QStringLiteral("return"), KEY_ENTER},
        {QStringLiteral("space"), KEY_SPACE},
        {QStringLiteral("backspace"), KEY_BACKSPACE},
        {QStringLiteral("delete"), KEY_DELETE},
        {QStringLiteral("insert"), KEY_INSERT},
        {QStringLiteral("home"), KEY_HOME},
        {QStringLiteral("end"), KEY_END},
        {QStringLiteral("pageup"), KEY_PAGEUP},
        {QStringLiteral("pagedown"), KEY_PAGEDOWN},
        {QStringLiteral("left"), KEY_LEFT},
        {QStringLiteral("right"), KEY_RIGHT},
        {QStringLiteral("up"), KEY_UP},
        {QStringLiteral("down"), KEY_DOWN},
        {QStringLiteral("f1"), KEY_F1},
        {QStringLiteral("f2"), KEY_F2},
        {QStringLiteral("f3"), KEY_F3},
        {QStringLiteral("f4"), KEY_F4},
        {QStringLiteral("f5"), KEY_F5},
        {QStringLiteral("f6"), KEY_F6},
        {QStringLiteral("f7"), KEY_F7},
        {QStringLiteral("f8"), KEY_F8},
        {QStringLiteral("f9"), KEY_F9},
        {QStringLiteral("f10"), KEY_F10},
        {QStringLiteral("f11"), KEY_F11},
        {QStringLiteral("f12"), KEY_F12},
        {QStringLiteral("minus"), KEY_MINUS},
        {QStringLiteral("equal"), KEY_EQUAL},
        {QStringLiteral("slash"), KEY_SLASH},
        {QStringLiteral("print"), KEY_SYSRQ},
    };
    return names.value(name, 0);
}

uint modifierForName(const QString &name)
{
    if (name == QLatin1String("ctrl") || name == QLatin1String("control")) {
        return InputContext::ControlModifier;
    }
    if (name == QLatin1String("shift")) {
        return InputContext::ShiftModifier;
    }
    if (name == QLatin1String("alt")) {
        return InputContext::AltModifier;
    }
    if (name == QLatin1String("super") || name == QLatin1String("meta")) {
        return InputContext::SuperModifier;
    }
    return 0;
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
        if (uint modifier = modifierForName(name)) {
            modifiers |= modifier;
        } else {
            code = evdevForName(name);
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
