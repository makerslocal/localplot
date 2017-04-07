#include "eta.h"

ExtEta::ExtEta(hpglListModel * model)
{
    hpglModel = model;
}

ExtEta::~ExtEta()
{
    //
}

double ExtEta::speedTranslate(int setting_speed)
{
//    return((0.5*setting_speed) + 30);
    return((0.3*setting_speed) + 70);
//    return((0.52*setting_speed) + 24.8);
}

/**
 * @brief AncillaryThread::lenHyp
 * @return - the length (in mm) of the hypotenuse of the command's line segments
 */
double ExtEta::lenHyp(const QPolygonF _poly)
{
    QPointF prev;
    QPointF curr;
    double mm = 0;
    prev.setX(0);
    prev.setY(0);

    for (int i = 0; i < _poly.length(); i++)
    {
        qreal x, y;
        curr = _poly.at(i);
        x = qFabs(curr.x() - prev.x());
        y = qFabs(curr.y() - prev.y());
        mm += qSqrt(x*x + y*y);
        prev.setX(curr.x());
        prev.setY(curr.y());
    }

    mm = mm * 0.025; // convert graphics units to mm
    return(mm);
}

double ExtEta::plotTime(const QPolygonF _poly)
{
    double retval = 0;
    QSettings settings;

    retval = lenHyp(_poly);

    if (retval <= 0)
    {
        return(retval);
    }

    retval = retval / speedTranslate(settings.value("device/speed/cut", SETDEF_DEVICE_SPEED_CUT).toInt());

    return(retval);
}

double ExtEta::plotTime(const QLineF _line)
{
    double retval = 0;
    QSettings settings;

    retval = _line.length() * 0.025;

    if (retval <= 0)
    {
        return(retval);
    }

    retval = retval / speedTranslate(settings.value("device/speed/travel", SETDEF_DEVICE_SPEED_TRAVEL).toInt());

    return(retval);
}

void ExtEta::process()
{
    double time = 0;
    QModelIndex index;
    QGraphicsItemGroup * itemGroup;
    QVector<QGraphicsPolygonItem*> * items;
    QLineF pu_line;

    pu_line.setP1(QPointF(0, 0));

    for (int i = 0; i < hpglModel->rowCount(); ++i)
    {
        index = hpglModel->index(i);
        itemGroup = NULL;
        items = NULL;
        hpglModel->dataItemsGroup(index, itemGroup, items);
        hpglModel->mutexLock();

        if (itemGroup == NULL)
        {
            qDebug() << "Error: itemgroup is null in process().";
            return;
        }
        if (items == NULL)
        {
            qDebug() << "Error: items is null in process().";
            return;
        }

        for (int i2 = 0; i2 < items->length(); ++i2)
        {
            QPolygonF poly = items->at(i2)->polygon();
            pu_line.setP2(itemGroup->mapToScene(poly.first()));
//            qDebug() << pu_line;
            time += plotTime(pu_line);
            time += plotTime(poly);
//            qDebug() << "File: " << i << ", Object: " << i2 << ", time: " << time;
            pu_line.setP1(itemGroup->mapToScene(poly.last()));
        }
        hpglModel->mutexUnlock();
        emit progress((int)(100 * ((qreal)i / (hpglModel->rowCount()-1))));
    }
    emit finished(time);
}

void ExtEta::statusUpdate(QString _consoleStatus)
{
    emit statusUpdate(_consoleStatus, Qt::black);
}
