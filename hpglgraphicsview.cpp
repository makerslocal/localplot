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
        QTransform oldtransform = transform();
        QTransform scaleTransform;
        scaleTransform.scale((1.0+(5.0/delta)), (1.0+(5.0/delta)));
        setTransform(oldtransform * scaleTransform);
    }
    else
    {
        event->ignore();
    }
    QGraphicsView::wheelEvent(event);
}
