#include "keyboardsurface.h"

#include <QMouseEvent>
#include <QTouchEvent>

namespace
{
constexpr int MouseId = -1;
}

KeyboardSurface::KeyboardSurface(QQuickItem *parent)
    : QQuickItem(parent)
    , m_model(new KeyModel(this))
{
    KeyGestures::Keys keys {
        [this](QPointF point) { return keyAt(point.x(), point.y()); },
        [this](int index) { return keyData(index); },
        [this] { return m_unitWidth; },
    };
    m_gestures = new KeyGestures(std::move(keys), this);
    connect(m_gestures, &KeyGestures::pressedChanged, this, &KeyboardSurface::refreshPressed);
    setAcceptTouchEvents(true);
    setAcceptedMouseButtons(Qt::LeftButton);
}

QVariantMap KeyboardSurface::page() const
{
    return m_page;
}

void KeyboardSurface::setPage(const QVariantMap &page)
{
    if (page == m_page) {
        return;
    }
    m_gestures->cancelAll();
    m_page = page;
    rebuild();
    Q_EMIT pageChanged();
}

QAbstractItemModel *KeyboardSurface::model() const
{
    return m_model;
}

KeyGestures *KeyboardSurface::gestures() const
{
    return m_gestures;
}

int KeyboardSurface::count() const
{
    return int(m_entries.size());
}

int KeyboardSurface::rowCount() const
{
    return int(m_page.value(QStringLiteral("rows")).toList().size());
}

qreal KeyboardSurface::gap() const
{
    return m_gap;
}

void KeyboardSurface::setGap(qreal gap)
{
    if (qFuzzyCompare(gap, m_gap)) {
        return;
    }
    m_gap = gap;
    relayout();
    Q_EMIT gapChanged();
}

bool KeyboardSurface::split() const
{
    return m_split;
}

void KeyboardSurface::setSplit(bool split)
{
    if (split == m_split) {
        return;
    }
    m_gestures->cancelAll();
    m_split = split;
    rebuild();
    Q_EMIT splitChanged();
}

qreal KeyboardSurface::splitGap() const
{
    return m_splitGap;
}

void KeyboardSurface::setSplitGap(qreal gap)
{
    if (qFuzzyCompare(gap, m_splitGap)) {
        return;
    }
    m_splitGap = gap;
    relayout();
    Q_EMIT splitGapChanged();
}

bool KeyboardSurface::interactive() const
{
    return m_interactive;
}

void KeyboardSurface::setInteractive(bool interactive)
{
    if (interactive == m_interactive) {
        return;
    }
    m_interactive = interactive;
    if (!interactive) {
        m_gestures->cancelAll();
    }
    Q_EMIT interactiveChanged();
}

qreal KeyboardSurface::unitWidth() const
{
    return m_unitWidth;
}

qreal KeyboardSurface::halfWidth() const
{
    return KeyLayout::halfWidth(params());
}

QVariantList KeyboardSurface::halfRects() const
{
    return KeyLayout::halfRects(params());
}

KeyLayout::Params KeyboardSurface::params() const
{
    return {width(), height(), m_gap, m_split, m_splitGap};
}

void KeyboardSurface::rebuild()
{
    m_entries = KeyLayout::compute(m_page, params());
    m_visualPressed.clear();
    m_model->reset(m_entries);
    relayout();
    Q_EMIT layoutChanged();
}

void KeyboardSurface::relayout()
{
    m_entries = KeyLayout::compute(m_page, params());
    m_model->updateGeometry(m_entries);
    const int columns = qMax(1, m_page.value(QStringLiteral("columns")).toInt());
    m_unitWidth = width() / columns;
    for (const KeyEntry &entry : std::as_const(m_entries)) {
        if (entry.key.value(QStringLiteral("letter")).toBool()) {
            m_unitWidth = entry.rect.width() + m_gap;
            break;
        }
    }
    Q_EMIT geometryUpdated();
}

void KeyboardSurface::refreshPressed()
{
    QList<int> pressed = m_gestures->pressedIndexes() + m_visualPressed;
    for (int i = 0; i < m_model->rowCount(); ++i) {
        m_model->setPressed(i, pressed.contains(i));
    }
}

void KeyboardSurface::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    QQuickItem::geometryChange(newGeometry, oldGeometry);
    if (newGeometry.size() != oldGeometry.size()) {
        relayout();
    }
}

int KeyboardSurface::keyAt(qreal x, qreal y) const
{
    const QPointF point(qBound<qreal>(0, x, qMax<qreal>(0, width() - 0.01)), qBound<qreal>(0, y, qMax<qreal>(0, height() - 0.01)));
    return KeyLayout::keyAt(m_entries, point);
}

QVariantMap KeyboardSurface::keyData(int index) const
{
    return index >= 0 && index < m_entries.size() ? m_entries.at(index).key : QVariantMap();
}

QRectF KeyboardSurface::keyRect(int index) const
{
    return index >= 0 && index < m_entries.size() ? m_entries.at(index).rect : QRectF();
}

int KeyboardSurface::neighbor(int index, int dx, int dy) const
{
    return KeyLayout::neighbor(m_entries, index, dx, dy);
}

int KeyboardSurface::indexOf(const QString &label) const
{
    int caseless = -1;
    for (int i = 0; i < m_entries.size(); ++i) {
        const QString candidate = m_entries.at(i).key.value(QStringLiteral("label")).toString();
        if (candidate == label) {
            return i;
        }
        if (caseless < 0 && candidate.compare(label, Qt::CaseInsensitive) == 0) {
            caseless = i;
        }
    }
    return caseless;
}

int KeyboardSurface::indexOfType(const QString &type) const
{
    for (int i = 0; i < m_entries.size(); ++i) {
        if (m_entries.at(i).key.value(QStringLiteral("type")).toString() == type) {
            return i;
        }
    }
    return -1;
}

QVariantList KeyboardSurface::glideKeys() const
{
    QVariantList keys;
    for (const KeyEntry &entry : m_entries) {
        if (entry.key.value(QStringLiteral("letter")).toBool()) {
            keys.append(QVariantMap {
                {QStringLiteral("label"), entry.key.value(QStringLiteral("label"))},
                {QStringLiteral("x"), entry.rect.x()},
                {QStringLiteral("y"), entry.rect.y()},
                {QStringLiteral("width"), entry.rect.width()},
                {QStringLiteral("height"), entry.rect.height()},
            });
        }
    }
    return keys;
}

QVariantList KeyboardSurface::glidePath(const QString &word) const
{
    QVariantList points;
    for (const QChar ch : word.toLower()) {
        const int index = indexOf(QString(ch));
        if (index >= 0) {
            const QPointF center = m_entries.at(index).rect.center();
            points.append(QVariantMap {{QStringLiteral("x"), center.x()}, {QStringLiteral("y"), center.y()}});
        }
    }
    return points;
}

void KeyboardSurface::setVisualPressed(int index, bool pressed)
{
    m_visualPressed.removeAll(index);
    if (pressed) {
        m_visualPressed.append(index);
    }
    refreshPressed();
}

void KeyboardSurface::touchEvent(QTouchEvent *event)
{
    for (const QEventPoint &point : event->points()) {
        if (point.state() == QEventPoint::Pressed && m_interactive && m_gestures->press(point.id(), point.position())) {
            event->setExclusiveGrabber(point, this);
        } else if (point.state() == QEventPoint::Updated) {
            m_gestures->move(point.id(), point.position());
        } else if (point.state() == QEventPoint::Released) {
            m_gestures->release(point.id(), point.position());
        }
    }
    event->accept();
}

void KeyboardSurface::touchUngrabEvent()
{
    m_gestures->cancelAll();
}

void KeyboardSurface::mousePressEvent(QMouseEvent *event)
{
    const bool real = event->source() == Qt::MouseEventNotSynthesized;
    event->setAccepted(real && m_interactive && m_gestures->press(MouseId, event->position()));
}

void KeyboardSurface::mouseMoveEvent(QMouseEvent *event)
{
    m_gestures->move(MouseId, event->position());
}

void KeyboardSurface::mouseReleaseEvent(QMouseEvent *event)
{
    m_gestures->release(MouseId, event->position());
}

void KeyboardSurface::mouseUngrabEvent()
{
    m_gestures->cancel(MouseId);
}
