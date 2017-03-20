/**
 * HPGL Graphics View - header
 * Christopher Bero <bigbero@gmail.com>
 */
#ifndef HPGLGRAPHICSVIEW_H
#define HPGLGRAPHICSVIEW_H

#include <QtCore>
#include <QApplication>
#include <QGraphicsView>
#include <QMouseEvent>
#include <QTransform>

namespace std {
class hpglGraphicsView;
}

class hpglGraphicsView : public QGraphicsView
{
    Q_OBJECT

public:
    using QGraphicsView::QGraphicsView;

signals:
    void mouseReleased();

protected:
    void mouseReleaseEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
};

#endif // HPGLGRAPHICSVIEW_H
