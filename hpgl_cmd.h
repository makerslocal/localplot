/**
 * HPGL_CMD - header
 * Christopher Bero <bigbero@gmail.com>
 */
#ifndef HPGLCMD_H
#define HPGLCMD_H

#include <math.h>

#include <QtCore>

namespace std {
class hpgl_cmd;
}

class hpgl_cmd
{
public:
    hpgl_cmd();
    ~hpgl_cmd();
    int set_opcode(QString _opcode);
    int set_pen(int _pen);
    QString get_opcode();
    int get_pen();
    QList<QPoint> get_coordList();

    void add_coord(QPoint _coord);
    double lenHyp();
    double lenX();
    double lenY();
    QString print();
    double time();

private:
    QString opcode;
    QList<QPoint> coordList;
    int pen;
};

#endif // HPGLCMD_H
