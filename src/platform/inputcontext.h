/*
    SPDX-FileCopyrightText: 2024 Aleix Pol i Gonzalez <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2024 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2026 DevL0rd <dmhzmxn@gmail.com>

    SPDX-License-Identifier: GPL-3.0-only
*/

#pragma once

#include <QObject>
#include <QQmlEngine>
#include <memory>

class InputMethod;
class InputMethodContext;

class InputContext : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(bool active READ isActive NOTIFY activeChanged)
    Q_PROPERTY(QString surroundingText READ surroundingText NOTIFY surroundingTextChanged)
    Q_PROPERTY(int cursorPosition READ cursorPosition NOTIFY surroundingTextChanged)
    Q_PROPERTY(int anchorPosition READ anchorPosition NOTIFY surroundingTextChanged)
    Q_PROPERTY(QString textBeforeCursor READ textBeforeCursor NOTIFY surroundingTextChanged)
    Q_PROPERTY(QString textAfterCursor READ textAfterCursor NOTIFY surroundingTextChanged)
    Q_PROPERTY(bool hasSurroundingText READ hasSurroundingText NOTIFY surroundingTextChanged)
    Q_PROPERTY(int contentHint READ contentHint NOTIFY contentTypeChanged)
    Q_PROPERTY(int contentPurpose READ contentPurpose NOTIFY contentTypeChanged)
    Q_PROPERTY(bool sensitive READ isSensitive NOTIFY contentTypeChanged)
    Q_PROPERTY(QString preferredLanguage READ preferredLanguage NOTIFY preferredLanguageChanged)
    Q_PROPERTY(QString preedit READ preedit NOTIFY preeditChanged)

public:
    enum ContentHint
    {
        content_hint_none = 0x0,
        content_hint_default = 0x7,
        content_hint_password = 0xc0,
        content_hint_auto_completion = 0x1,
        content_hint_auto_correction = 0x2,
        content_hint_auto_capitalization = 0x4,
        content_hint_lowercase = 0x8,
        content_hint_uppercase = 0x10,
        content_hint_titlecase = 0x20,
        content_hint_hidden_text = 0x40,
        content_hint_sensitive_data = 0x80,
        content_hint_latin = 0x100,
        content_hint_multiline = 0x200,
    };
    Q_ENUM(ContentHint)

    enum ContentPurpose
    {
        content_purpose_normal = 0,
        content_purpose_alpha = 1,
        content_purpose_digits = 2,
        content_purpose_number = 3,
        content_purpose_phone = 4,
        content_purpose_url = 5,
        content_purpose_email = 6,
        content_purpose_name = 7,
        content_purpose_password = 8,
        content_purpose_date = 9,
        content_purpose_time = 10,
        content_purpose_datetime = 11,
        content_purpose_terminal = 12,
    };
    Q_ENUM(ContentPurpose)

    enum Modifier
    {
        ShiftModifier = 0x1,
        LockModifier = 0x2,
        ControlModifier = 0x4,
        AltModifier = 0x8,
        SuperModifier = 0x40,
    };
    Q_ENUM(Modifier)

    static InputContext *instance();
    static InputContext *create(QQmlEngine *, QJSEngine *);

    bool isActive() const;
    QString surroundingText() const;
    int cursorPosition() const;
    int anchorPosition() const;
    QString textBeforeCursor() const;
    QString textAfterCursor() const;
    bool hasSurroundingText() const;
    int contentHint() const;
    int contentPurpose() const;
    bool isSensitive() const;
    QString preferredLanguage() const;
    QString preedit() const;

    Q_INVOKABLE void commit(const QString &text);
    Q_INVOKABLE void setPreedit(const QString &text, int cursor = -1);
    Q_INVOKABLE void commitPreedit();
    Q_INVOKABLE void clearPreedit();
    Q_INVOKABLE void deleteSurrounding(int charsBefore, int charsAfter);
    Q_INVOKABLE void backspace();
    Q_INVOKABLE void deleteWordBefore();
    Q_INVOKABLE void enter();
    Q_INVOKABLE void tab();
    Q_INVOKABLE void keysym(uint sym, uint modifiers = 0);
    Q_INVOKABLE void key(uint evdevCode, uint modifiers = 0);
    Q_INVOKABLE void keyDown(uint evdevCode, uint modifiers = 0);
    Q_INVOKABLE void keyUp(uint evdevCode);
    Q_INVOKABLE bool shortcut(const QString &combo);
    Q_INVOKABLE void moveCursor(int steps);
    Q_INVOKABLE void moveCursorVertical(int steps);

Q_SIGNALS:
    void activeChanged();
    void activated();
    void deactivated();
    void surroundingTextChanged();
    void contentTypeChanged();
    void preferredLanguageChanged();
    void preeditChanged();
    void resetRequested();
    void committed(const QString &text);

private:
    explicit InputContext(QObject *parent = nullptr);
    void attach();
    void sendModifiers(uint modifiers);
    int byteOffsetToChar(uint32_t bytes) const;

    InputMethod *m_inputMethod;
    std::shared_ptr<InputMethodContext> m_context;
    QString m_preedit;
    QString m_language;
};
