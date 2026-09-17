#include "focusdriver.h"

#include <QCoreApplication>
#include <QKeyEvent>
#include <QQuickWindow>

FocusDriver::FocusDriver(QObject *parent)
    : QObject(parent)
{ }

void FocusDriver::sendKey(QQuickItem *anyItem, int key, int modifiers)
{
    if (!anyItem || !anyItem->window()) {
        return;
    }
    QQuickWindow *window = anyItem->window();
    QKeyEvent press(QEvent::KeyPress, key, Qt::KeyboardModifiers(modifiers));
    QCoreApplication::sendEvent(window, &press);
    QKeyEvent release(QEvent::KeyRelease, key, Qt::KeyboardModifiers(modifiers));
    QCoreApplication::sendEvent(window, &release);
}

bool FocusDriver::moveFocus(QQuickItem *anyItem, bool forward)
{
    if (!anyItem || !anyItem->window()) {
        return false;
    }
    QQuickItem *current = anyItem->window()->activeFocusItem();
    if (!current) {
        current = anyItem->window()->contentItem();
    }
    QQuickItem *next = current->nextItemInFocusChain(forward);
    if (!next || next == current) {
        return false;
    }
    next->forceActiveFocus(forward ? Qt::TabFocusReason : Qt::BacktabFocusReason);
    return true;
}
