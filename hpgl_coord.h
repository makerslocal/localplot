#ifndef HPGL_COORD_H
#define HPGL_COORD_H

#include <QDebug>
#include <QString>

namespace std {
class hpgl_coord;
}

class hpgl_coord
{
public:
    hpgl_coord();
    hpgl_coord(int newX, int newY);
    ~hpgl_coord();
    int get_x();
    int get_y();
    void set_x(int newX);
    void set_y(int newY);
    QString print();

public slots:
    //int ();

protected:
    int x;
    int y;
};

#endif // HPGL_COORD_H
