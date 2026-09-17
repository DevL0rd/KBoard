#pragma once

#include "keygestures.h"
#include "keylayout.h"
#include "keymodel.h"

#include <QQuickItem>

class KeyboardSurface : public QQuickItem
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QVariantMap page READ page WRITE setPage NOTIFY pageChanged)
    Q_PROPERTY(QAbstractItemModel *model READ model CONSTANT)
    Q_PROPERTY(KeyGestures *gestures READ gestures CONSTANT)
    Q_PROPERTY(int count READ count NOTIFY layoutChanged)
    Q_PROPERTY(int rowCount READ rowCount NOTIFY layoutChanged)
    Q_PROPERTY(qreal gap READ gap WRITE setGap NOTIFY gapChanged)
    Q_PROPERTY(bool split READ split WRITE setSplit NOTIFY splitChanged)
    Q_PROPERTY(qreal splitGap READ splitGap WRITE setSplitGap NOTIFY splitGapChanged)
    Q_PROPERTY(bool interactive READ interactive WRITE setInteractive NOTIFY interactiveChanged)
    Q_PROPERTY(qreal unitWidth READ unitWidth NOTIFY geometryUpdated)
    Q_PROPERTY(qreal halfWidth READ halfWidth NOTIFY geometryUpdated)
    Q_PROPERTY(QVariantList halfRects READ halfRects NOTIFY geometryUpdated)

public:
    explicit KeyboardSurface(QQuickItem *parent = nullptr);

    QVariantMap page() const;
    void setPage(const QVariantMap &page);
    QAbstractItemModel *model() const;
    KeyGestures *gestures() const;
    int count() const;
    int rowCount() const;
    qreal gap() const;
    void setGap(qreal gap);
    bool split() const;
    void setSplit(bool split);
    qreal splitGap() const;
    void setSplitGap(qreal gap);
    bool interactive() const;
    void setInteractive(bool interactive);
    qreal unitWidth() const;
    qreal halfWidth() const;
    QVariantList halfRects() const;

    Q_INVOKABLE int keyAt(qreal x, qreal y) const;
    Q_INVOKABLE QVariantMap keyData(int index) const;
    Q_INVOKABLE QRectF keyRect(int index) const;
    Q_INVOKABLE int neighbor(int index, int dx, int dy) const;
    Q_INVOKABLE int indexOf(const QString &label) const;
    Q_INVOKABLE int indexOfType(const QString &type) const;
    Q_INVOKABLE QVariantList glideKeys() const;
    Q_INVOKABLE QVariantList glidePath(const QString &word) const;
    Q_INVOKABLE void setVisualPressed(int index, bool pressed);

Q_SIGNALS:
    void pageChanged();
    void layoutChanged();
    void gapChanged();
    void splitChanged();
    void splitGapChanged();
    void interactiveChanged();
    void geometryUpdated();

protected:
    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;
    void touchEvent(QTouchEvent *event) override;
    void touchUngrabEvent() override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseUngrabEvent() override;

private:
    KeyLayout::Params params() const;
    void rebuild();
    void relayout();
    void refreshPressed();

    QVariantMap m_page;
    KeyModel *m_model;
    KeyGestures *m_gestures;
    QList<KeyEntry> m_entries;
    QList<int> m_visualPressed;
    qreal m_gap = 6;
    bool m_split = false;
    qreal m_splitGap = 0;
    bool m_interactive = true;
    qreal m_unitWidth = 0;
};
