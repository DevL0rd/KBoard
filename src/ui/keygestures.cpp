#include "keygestures.h"

#include <cmath>

namespace
{
constexpr qreal CursorThreshold = 0.45;
constexpr qreal CursorStep = 0.3;
constexpr qreal DeleteThreshold = 0.6;
constexpr qreal DeleteWordTravel = 0.7;
constexpr qreal GlideThreshold = 0.55;
constexpr qreal GlidePointSpacing = 2.0;

QString typeOf(const QVariantMap &key)
{
    return key.value(QStringLiteral("type")).toString();
}

bool rollsOver(const QVariantMap &key)
{
    const QString type = typeOf(key);
    return type == QLatin1String("char") || type == QLatin1String("space");
}
}

struct KeyGestures::Track
{
    int id = 0;
    int index = -1;
    QPointF start;
    QPointF last;
    QString mode = QStringLiteral("press");
    QTimer longPress;
    QTimer repeat;
    int repeatCount = 0;
    qreal cursorOrigin = 0;
    int deleteWords = 0;
    QVariantList points;
    QElapsedTimer clock;
};

KeyGestures::KeyGestures(Keys keys, QObject *parent)
    : QObject(parent)
    , m_keys(std::move(keys))
{ }

KeyGestures::~KeyGestures() = default;

int KeyGestures::activeTouches() const
{
    return int(m_tracks.size());
}

QList<int> KeyGestures::pressedIndexes() const
{
    QList<int> indexes;
    for (const auto &entry : m_tracks) {
        if (entry.second->mode != QLatin1String("glide")) {
            indexes.append(entry.second->index);
        }
    }
    return indexes;
}

KeyGestures::Track *KeyGestures::track(int id) const
{
    const auto it = m_tracks.find(id);
    return it == m_tracks.end() ? nullptr : it->second.get();
}

qreal KeyGestures::unit() const
{
    return qMax<qreal>(1, m_keys.unitWidth());
}

QVariantMap KeyGestures::pointAt(const Track *track, QPointF pos) const
{
    return {{QStringLiteral("x"), pos.x()}, {QStringLiteral("y"), pos.y()}, {QStringLiteral("t"), track->clock.elapsed()}};
}

void KeyGestures::setTouchMode(int touchId, const QString &mode)
{
    Track *t = track(touchId);
    if (!t) {
        return;
    }
    t->mode = mode;
    t->longPress.stop();
    t->repeat.stop();
    Q_EMIT pressedChanged();
}

void KeyGestures::startTimers(Track *track)
{
    track->longPress.stop();
    track->repeat.stop();
    if (m_keys.keyData(track->index).value(QStringLiteral("repeat")).toBool()) {
        track->repeat.start(m_repeatDelay);
    } else {
        track->longPress.start(m_longPressDelay);
    }
}

void KeyGestures::rollOver()
{
    struct Rolled
    {
        int id;
        int index;
        QPointF last;
    };
    QList<Rolled> rolled;
    for (const auto &entry : m_tracks) {
        Track *other = entry.second.get();
        if (other->mode == QLatin1String("press") && rollsOver(m_keys.keyData(other->index))) {
            other->mode = QStringLiteral("done");
            other->longPress.stop();
            rolled.append({other->id, other->index, other->last});
        }
    }
    for (const Rolled &other : std::as_const(rolled)) {
        Q_EMIT keyReleased(other.id, other.index, other.last.x(), other.last.y(), QStringLiteral("tap"));
    }
}

bool KeyGestures::press(int id, QPointF pos)
{
    const int index = m_keys.keyAt(pos);
    if (index < 0) {
        return false;
    }
    cancel(id);
    rollOver();

    auto created = std::make_unique<Track>();
    Track *t = created.get();
    t->id = id;
    t->index = index;
    t->start = pos;
    t->last = pos;
    t->clock.start();
    t->longPress.setSingleShot(true);
    t->repeat.setSingleShot(true);
    connect(&t->longPress, &QTimer::timeout, this, [this, id] { onLongPress(id); });
    connect(&t->repeat, &QTimer::timeout, this, [this, id] { onRepeat(id); });
    m_tracks[id] = std::move(created);
    startTimers(t);
    Q_EMIT activeTouchesChanged();
    Q_EMIT pressedChanged();
    Q_EMIT keyPressed(id, index, pos.x(), pos.y());
    return true;
}

void KeyGestures::move(int id, QPointF pos)
{
    Track *t = track(id);
    if (!t) {
        return;
    }
    t->last = pos;
    if (t->mode == QLatin1String("press")) {
        movePressed(t, pos);
    } else {
        moveGesture(t, pos);
    }
}

void KeyGestures::movePressed(Track *track, QPointF pos)
{
    const QPointF delta = pos - track->start;
    if (beginCursor(track, delta) || beginDelete(track, delta) || beginGlide(track, pos)) {
        return;
    }
    slidePress(track, pos);
}

bool KeyGestures::beginCursor(Track *track, QPointF delta)
{
    if (!m_cursorSwipeEnabled || typeOf(m_keys.keyData(track->index)) != QLatin1String("space")
        || std::abs(delta.x()) <= unit() * CursorThreshold) {
        return false;
    }
    track->mode = QStringLiteral("cursor");
    track->longPress.stop();
    track->cursorOrigin = track->start.x() + std::copysign(unit() * CursorThreshold, delta.x());
    moveGesture(track, track->last);
    return true;
}

bool KeyGestures::beginDelete(Track *track, QPointF delta)
{
    const bool backspace = typeOf(m_keys.keyData(track->index)) == QLatin1String("backspace");
    if (!m_deleteSwipeEnabled || !backspace || track->repeatCount > 0 || delta.x() >= -unit() * DeleteThreshold) {
        return false;
    }
    track->mode = QStringLiteral("delete");
    track->repeat.stop();
    moveGesture(track, track->last);
    return true;
}

bool KeyGestures::beginGlide(Track *track, QPointF pos)
{
    const bool glideKey = m_keys.keyData(track->index).value(QStringLiteral("glide")).toBool();
    const QPointF delta = pos - track->start;
    if (!m_glideEnabled || !glideKey || m_tracks.size() != 1 || std::hypot(delta.x(), delta.y()) <= unit() * GlideThreshold
        || m_keys.keyAt(pos) == track->index) {
        return false;
    }
    track->mode = QStringLiteral("glide");
    track->longPress.stop();
    track->points = {pointAt(track, track->start), pointAt(track, pos)};
    Q_EMIT pressedChanged();
    Q_EMIT glideStarted(track->id, track->start.x(), track->start.y());
    Q_EMIT glideMoved(pos.x(), pos.y());
    return true;
}

void KeyGestures::slidePress(Track *track, QPointF pos)
{
    const QVariantMap key = m_keys.keyData(track->index);
    const QString type = typeOf(key);
    const bool pinned = (key.value(QStringLiteral("glide")).toBool() && m_glideEnabled) || type == QLatin1String("space")
        || type == QLatin1String("backspace");
    const int index = m_keys.keyAt(pos);
    if (pinned || track->repeatCount > 0 || index < 0 || index == track->index) {
        return;
    }
    const int from = track->index;
    track->index = index;
    startTimers(track);
    Q_EMIT pressedChanged();
    Q_EMIT pressMoved(track->id, from, index);
}

void KeyGestures::moveGesture(Track *track, QPointF pos)
{
    if (track->mode == QLatin1String("popup")) {
        Q_EMIT touchMoved(track->id, pos.x(), pos.y());
    } else if (track->mode == QLatin1String("glide")) {
        const QVariantMap last = track->points.constLast().toMap();
        if (std::hypot(last.value(QStringLiteral("x")).toReal() - pos.x(), last.value(QStringLiteral("y")).toReal() - pos.y())
            >= GlidePointSpacing) {
            track->points.append(pointAt(track, pos));
        }
        Q_EMIT glideMoved(pos.x(), pos.y());
    } else if (track->mode == QLatin1String("cursor")) {
        const qreal step = unit() * CursorStep;
        const int steps = int((pos.x() - track->cursorOrigin) / step);
        if (steps != 0) {
            track->cursorOrigin += steps * step;
            Q_EMIT cursorSwipe(steps);
        }
    } else if (track->mode == QLatin1String("delete")) {
        const qreal travel = track->start.x() - unit() * DeleteThreshold - pos.x();
        const int words = travel < 0 ? 0 : int(travel / (unit() * DeleteWordTravel)) + 1;
        if (words != track->deleteWords) {
            track->deleteWords = words;
            Q_EMIT deleteSwipe(words);
        }
    }
}

void KeyGestures::release(int id, QPointF pos)
{
    move(id, pos);
    const auto it = m_tracks.find(id);
    if (it == m_tracks.end()) {
        return;
    }
    std::unique_ptr<Track> t = std::move(it->second);
    m_tracks.erase(it);
    finish(std::move(t), pos, false);
}

void KeyGestures::cancel(int id)
{
    const auto it = m_tracks.find(id);
    if (it == m_tracks.end()) {
        return;
    }
    std::unique_ptr<Track> t = std::move(it->second);
    m_tracks.erase(it);
    const QPointF last = t->last;
    finish(std::move(t), last, true);
}

void KeyGestures::cancelAll()
{
    while (!m_tracks.empty()) {
        cancel(m_tracks.begin()->first);
    }
}

void KeyGestures::finish(std::unique_ptr<Track> track, QPointF pos, bool cancelled)
{
    track->longPress.stop();
    track->repeat.stop();
    Q_EMIT activeTouchesChanged();
    Q_EMIT pressedChanged();
    const QString &mode = track->mode;
    if (mode == QLatin1String("glide")) {
        track->points.append(pointAt(track.get(), pos));
        cancelled ? Q_EMIT glideCancelled() : Q_EMIT glideFinished(track->points);
    } else if (mode == QLatin1String("cursor")) {
        Q_EMIT cursorSwipeFinished();
    } else if (mode == QLatin1String("delete")) {
        Q_EMIT deleteSwipeFinished(cancelled ? 0 : track->deleteWords);
    } else {
        const QString reported = cancelled ? QStringLiteral("cancel") : mode == QLatin1String("press") ? QStringLiteral("tap") : mode;
        Q_EMIT keyReleased(track->id, track->index, pos.x(), pos.y(), reported);
    }
}

void KeyGestures::onLongPress(int id)
{
    Track *t = track(id);
    if (t && t->mode == QLatin1String("press")) {
        Q_EMIT keyLongPressed(id, t->index);
    }
}

void KeyGestures::onRepeat(int id)
{
    Track *t = track(id);
    if (!t || (t->mode != QLatin1String("press") && t->mode != QLatin1String("repeat"))) {
        return;
    }
    t->mode = QStringLiteral("repeat");
    ++t->repeatCount;
    const qreal next = std::max<qreal>(m_repeatInterval * 0.4, m_repeatInterval * 2.2 * std::pow(0.8, t->repeatCount));
    t->repeat.start(int(next));
    Q_EMIT keyRepeated(id, t->index, t->repeatCount);
}
