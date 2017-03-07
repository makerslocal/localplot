#include "hpglpolygon.h"

QVariant hpglQGraphicsPolygonItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemSelectedChange)
    {
            if (value == true)
            {
                qDebug() << "FOO";
            }
            else
            {
                //
            }
    }
    return (itemChange(change, value));
}
