/**
 * HPGL QPolygon - header
 * Christopher Bero <bigbero@gmail.com>
 */
#ifndef HPGLPOLYGON_H
#define HPGLPOLYGON_H

#include <QtCore>
#include <QGraphicsPolygonItem>

namespace std {
class hpglQGraphicsPolygonItem;
}

class hpglQGraphicsPolygonItem : public QGraphicsPolygonItem
{
public:
    using QGraphicsPolygonItem::QGraphicsPolygonItem;

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);
};

#endif // HPGLPOLYGON_H
