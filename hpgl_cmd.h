#ifndef HPGLCMD_H
#define HPGLCMD_H

#include <QList>
#include <QLine>
#include <QTransform>
#include <QPoint>
#include <QDebug>
#include <QString>

#include <string>

namespace std {
class hpgl_cmd;
}

class hpgl_cmd
{
public:
    hpgl_cmd();
    hpgl_cmd(QString newopcode, QList<QPoint> newList);
    ~hpgl_cmd();
    QString opcode();
    QList<QPoint> point_list();
    int get_pen();

public slots:
    //

protected:
    QString _opcode;
    QList<QPoint> coordList;
    int pen;
};

#endif // HPGLCMD_H
