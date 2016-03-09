#include "hpgl_coord.h"

hpgl_coord::hpgl_coord()
{
    x = 0;
    y = 0;
}

hpgl_coord::hpgl_coord(int newX, int newY)
{
    x = newX;
    y = newY;
}

hpgl_coord::~hpgl_coord()
{
    //
}

void hpgl_coord::set_x(int newX)
{
    x = newX;
}

void hpgl_coord::set_y(int newY)
{
    y = newY;
}

int hpgl_coord::get_x()
{
    return x;
}

int hpgl_coord::get_y()
{
    return y;
}

QString hpgl_coord::print()
{
    QString text;
    text = QString::number(x);
    text += ",";
    text += QString::number(y);
    //qDebug() << "printed string: " << text;
    return(text);
}
