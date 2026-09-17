#pragma once

#include <KConfigWatcher>

#include <QDBusServiceWatcher>
#include <QObject>
#include <QQmlEngine>

class SystemStatus : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(bool keyboardRunning READ keyboardRunning NOTIFY keyboardRunningChanged)
    Q_PROPERTY(bool kwinReachable READ kwinReachable NOTIFY virtualKeyboardChanged)
    Q_PROPERTY(bool virtualKeyboardAvailable READ virtualKeyboardAvailable NOTIFY virtualKeyboardChanged)
    Q_PROPERTY(int virtualKeyboardMode READ virtualKeyboardMode WRITE setVirtualKeyboardMode NOTIFY virtualKeyboardChanged)
    Q_PROPERTY(QString virtualKeyboardError READ virtualKeyboardError NOTIFY virtualKeyboardChanged)
    Q_PROPERTY(QString inputMethod READ inputMethod NOTIFY inputMethodChanged)
    Q_PROPERTY(bool kboardIsInputMethod READ kboardIsInputMethod NOTIFY inputMethodChanged)

public:
    enum Mode
    {
        Never = 0,
        TouchOnly = 1,
        AnyInput = 2,
    };
    Q_ENUM(Mode)

    explicit SystemStatus(QObject *parent = nullptr);

    bool keyboardRunning() const;
    bool kwinReachable() const;
    bool virtualKeyboardAvailable() const;
    int virtualKeyboardMode() const;
    void setVirtualKeyboardMode(int mode);
    QString virtualKeyboardError() const;
    QString inputMethod() const;
    bool kboardIsInputMethod() const;

    Q_INVOKABLE void openVirtualKeyboardSettings();

Q_SIGNALS:
    void keyboardRunningChanged();
    void virtualKeyboardChanged();
    void inputMethodChanged();

private Q_SLOTS:
    void refreshVirtualKeyboard();
    void onKWinPropertiesChanged(const QString &interface, const QVariantMap &changed, const QStringList &invalidated);

private:
    void readInputMethod();

    QDBusServiceWatcher m_keyboardWatcher;
    QDBusServiceWatcher m_kwinWatcher;
    KConfigWatcher::Ptr m_kwinConfigWatcher;
    bool m_keyboardRunning = false;
    bool m_kwinReachable = false;
    bool m_available = false;
    int m_mode = -1;
    QString m_error;
    QString m_inputMethod;
};
