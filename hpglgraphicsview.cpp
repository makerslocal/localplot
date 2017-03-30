#include "hpglgraphicsview.h"

void hpglGraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
//    qDebug() << "HGV button: " << event->button();
    if (event->button() == Qt::LeftButton)
    {
//        qDebug() << "HGV: Emitting mouseReleased.";
        emit mouseReleased();
    }
    // Carry on with default action
    QGraphicsView::mouseReleaseEvent(event);
}

void hpglGraphicsView::wheelEvent(QWheelEvent *event)
{
    if (QApplication::keyboardModifiers() == Qt::ControlModifier)
    {
        int delta = event->delta();
        emit zoomDelta(delta);
    }
    else
    {
        event->ignore();
    }
    QGraphicsView::wheelEvent(event);
}
