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
#include <QGraphicsItem>
#include <QGraphicsItemGroup>

#include "settings.h"

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
    void statusUpdate(QString text, QColor textColor);
    void zoomUpdate(QString text);

public slots:
    void setGrid();
    void zoomActual();
    void zoomSceneRect();
    void zoomGraphicsItems();
    void zoomSelectedItems();
    void zoomDelta(int delta);
    void zoomOut();
    void zoomIn();

protected:
    void mouseReleaseEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);

private:
    void statusUpdate(QString _consoleStatus);
};

#endif // HPGLGRAPHICSVIEW_H
