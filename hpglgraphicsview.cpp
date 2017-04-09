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
        zoomDelta(delta);
    }
    else
    {
        event->ignore();
    }
    QGraphicsView::wheelEvent(event);
}

void hpglGraphicsView::statusUpdate(QString _consoleStatus)
{
    emit statusUpdate(_consoleStatus, Qt::black);
}

void hpglGraphicsView::setGrid()
{
    return;

    QTransform _transform = transform();
    // physicalDpi is the number of pixels in an inch
    int xDpi = physicalDpiX();
    int yDpi = physicalDpiY();
    QSettings settings;
    if (!settings.value("mainwindow/grid", SETDEF_MAINWINDOW_GRID).toBool())
    {
        setBackgroundBrush(Qt::NoBrush);
        return;
    }
    int size = settings.value("mainwindow/grid/size", SETDEF_MAINWINDOW_GRID_SIZE).toInt();

    QTransform hpglToPx;
    hpglToPx.scale(xDpi/1016.0, yDpi/1016.0);

    // m11 and m22 are horizontal and vertical scaling
    qreal mx, my;
    mx = hpglToPx.m11() / qAbs(_transform.m11());
    my = hpglToPx.m22() / qAbs(_transform.m22());

    int gridX, gridY;
    if (settings.value("device/width/type", SETDEF_DEVICE_WDITH_TYPE).toInt() == deviceWidth_t::CM)
    {
        gridX = (((xDpi*size)/2.54) / mx);
        gridY = (((yDpi*size)/2.54) / my);
    }
    else
    {
        gridX = ((xDpi*size) / mx);
        gridY = ((yDpi*size) / my);
    }

    QImage grid(gridX, gridY, QImage::Format_RGB32);
    QRgb value;

    value = qRgb(100, 240, 100);

    for (int x = 0; x < gridX; ++x)
    {
        for (int y = 0; y < gridY; ++y)
        {
            grid.setPixelColor(QPoint(x, y), QColor(40, 40, 40));
        }
    }
    for (int i = 0; i < gridX; ++i)
    {
        grid.setPixel(i, 0, value);
    }
    for (int i = 0; i < gridY; ++i)
    {
        grid.setPixel(0, i, value);
    }

    QBrush gridBrush(grid);

    gridBrush.setTransform((_transform.inverted()));

    setBackgroundBrush(gridBrush);
//    plotScene.setBackgroundBrush(gridBrush);
}

void hpglGraphicsView::zoomActual()
{
    // physicalDpi is the number of pixels in an inch
    int xDpi = physicalDpiX();
    int yDpi = physicalDpiY();
    // Transforms
    QTransform hpglToPx, viewFlip;
    hpglToPx.scale(xDpi/1016.0, yDpi/1016.0);
    viewFlip.scale(1, -1);

    setTransform(hpglToPx * viewFlip);
    setGrid();

    emit statusUpdate("Scene scale set to 1:1", Qt::darkGreen);
    emit zoomUpdate("Actual size");
}

void hpglGraphicsView::zoomSceneRect()
{
    fitInView(
        scene()->sceneRect(),
        Qt::KeepAspectRatio);
    setGrid();
    emit statusUpdate("Scene scale set to view all", Qt::darkGreen);
    emit zoomUpdate("Vinyl width");
}

void hpglGraphicsView::zoomGraphicsItems()
{
    QRectF newrect;

    newrect.setX(0);
    newrect.setY(0);
    newrect.setWidth(0);
    newrect.setHeight(0);

    QList<QGraphicsItem*> itemList = items();
    // type() = 10 -> QGraphicsItemGroup

    for (int i = 0; i < itemList.length(); ++i)
    {
        QGraphicsItem * item = itemList.at(i);
        if (item->type() == 10) // item is a qgraphicsitemgroup
        {
            QRectF compRect = item->boundingRect();
            QPointF compPoint = item->pos();

            if ((compPoint.x()+compRect.width()) > newrect.width())
            {
                newrect.setWidth(compPoint.x()+compRect.width());
            }
            if ((compPoint.y()+compRect.height()) > newrect.height())
            {
                newrect.setHeight(compPoint.y()+compRect.height());
            }
        }
    }

    fitInView(newrect, Qt::KeepAspectRatio);
    setGrid();

    emit statusUpdate("Scene scale set to contain items", Qt::darkGreen);
    emit zoomUpdate("Show all items");
}

void hpglGraphicsView::zoomSelectedItems()
{
    QRectF newrect;

    newrect.setX(0);
    newrect.setY(0);
    newrect.setWidth(0);
    newrect.setHeight(0);

    QList<QGraphicsItem*> itemList = items();
    // type() = 10 -> QGraphicsItemGroup

    for (int i = 0; i < itemList.length(); ++i)
    {
        QGraphicsItem * item = itemList.at(i);
        if (item->type() == 10 && item->isSelected()) // item is a qgraphicsitemgroup
        {
            QRectF compRect = item->boundingRect();
            QPointF compPoint = item->pos();

            if ((compPoint.x()+compRect.width()) > newrect.width())
            {
                newrect.setWidth(compPoint.x()+compRect.width());
            }
            if ((compPoint.y()+compRect.height()) > newrect.height())
            {
                newrect.setHeight(compPoint.y()+compRect.height());
            }
        }
    }

    fitInView(newrect, Qt::KeepAspectRatio);
    setGrid();

    emit statusUpdate("Scene scale set to contain items", Qt::darkGreen);
    emit zoomUpdate("Show all items");
}

void hpglGraphicsView::zoomDelta(int delta)
{
    QTransform oldtransform = transform();
    QTransform scaleTransform;
    scaleTransform.scale((1.0+(5.0/delta)), (1.0+(5.0/delta)));
    setTransform(oldtransform * scaleTransform);
    setGrid();
    emit statusUpdate("Scene scale set to scroll wheel zoom.", Qt::darkGreen);
    emit zoomUpdate("Custom");
}

void hpglGraphicsView::zoomOut()
{
    zoomDelta(-30);
}

void hpglGraphicsView::zoomIn()
{
    zoomDelta(30);
}






























