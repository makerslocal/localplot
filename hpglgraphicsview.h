/**
 * HPGL Graphics View - header
 * Christopher Bero <bigbero@gmail.com>
 */
#ifndef HPGLGRAPHICSVIEW_H
#define HPGLGRAPHICSVIEW_H

#include <QtCore>
#include <QGraphicsView>
#include <QMouseEvent>

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
};

#endif // HPGLGRAPHICSVIEW_H
