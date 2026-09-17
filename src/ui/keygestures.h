#pragma once

#include <QElapsedTimer>
#include <QObject>
#include <QPointF>
#include <QQmlEngine>
#include <QTimer>
#include <QVariantMap>

#include <functional>
#include <map>
#include <memory>

class KeyGestures : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Owned by KeyboardSurface")

    Q_PROPERTY(int longPressDelay MEMBER m_longPressDelay NOTIFY configChanged)
    Q_PROPERTY(int repeatDelay MEMBER m_repeatDelay NOTIFY configChanged)
    Q_PROPERTY(int repeatInterval MEMBER m_repeatInterval NOTIFY configChanged)
    Q_PROPERTY(bool glideEnabled MEMBER m_glideEnabled NOTIFY configChanged)
    Q_PROPERTY(bool cursorSwipeEnabled MEMBER m_cursorSwipeEnabled NOTIFY configChanged)
    Q_PROPERTY(bool deleteSwipeEnabled MEMBER m_deleteSwipeEnabled NOTIFY configChanged)
    Q_PROPERTY(int activeTouches READ activeTouches NOTIFY activeTouchesChanged)

public:
    struct Keys
    {
        std::function<int(QPointF)> keyAt;
        std::function<QVariantMap(int)> keyData;
        std::function<qreal()> unitWidth;
    };

    explicit KeyGestures(Keys keys, QObject *parent = nullptr);
    ~KeyGestures() override;

    int activeTouches() const;
    QList<int> pressedIndexes() const;

    bool press(int id, QPointF pos);
    void move(int id, QPointF pos);
    void release(int id, QPointF pos);
    void cancel(int id);
    void cancelAll();

    Q_INVOKABLE void setTouchMode(int touchId, const QString &mode);

Q_SIGNALS:
    void configChanged();
    void activeTouchesChanged();
    void pressedChanged();

    void keyPressed(int touchId, int index, qreal x, qreal y);
    void keyReleased(int touchId, int index, qreal x, qreal y, const QString &mode);
    void pressMoved(int touchId, int from, int to);
    void keyLongPressed(int touchId, int index);
    void keyRepeated(int touchId, int index, int count);
    void touchMoved(int touchId, qreal x, qreal y);
    void glideStarted(int touchId, qreal x, qreal y);
    void glideMoved(qreal x, qreal y);
    void glideFinished(const QVariantList &points);
    void glideCancelled();
    void cursorSwipe(int steps);
    void cursorSwipeFinished();
    void deleteSwipe(int words);
    void deleteSwipeFinished(int words);

private:
    struct Track;

    Track *track(int id) const;
    void startTimers(Track *track);
    void rollOver();
    void movePressed(Track *track, QPointF pos);
    bool beginCursor(Track *track, QPointF delta);
    bool beginDelete(Track *track, QPointF delta);
    bool beginGlide(Track *track, QPointF pos);
    void slidePress(Track *track, QPointF pos);
    void moveGesture(Track *track, QPointF pos);
    void finish(std::unique_ptr<Track> track, QPointF pos, bool cancelled);
    void onLongPress(int id);
    void onRepeat(int id);
    QVariantMap pointAt(const Track *track, QPointF pos) const;
    qreal unit() const;

    Keys m_keys;
    std::map<int, std::unique_ptr<Track>> m_tracks;
    int m_longPressDelay = 300;
    int m_repeatDelay = 400;
    int m_repeatInterval = 45;
    bool m_glideEnabled = true;
    bool m_cursorSwipeEnabled = true;
    bool m_deleteSwipeEnabled = true;
};
